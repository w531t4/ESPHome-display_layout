#pragma once
#include "esphome.h"
extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}

class CoreLoadSensorsRTOS : public esphome::PollingComponent {
  public:
    esphome::Sensor *core0 = new esphome::Sensor();
    esphome::Sensor *core1 = new esphome::Sensor();

    explicit CoreLoadSensorsRTOS(uint32_t update_interval_ms)
        : PollingComponent(update_interval_ms) {}

    void update() override {
        // Snapshot system state
        UBaseType_t max_tasks = uxTaskGetNumberOfTasks() + 4;
        std::vector<TaskStatus_t> tasks(max_tasks);
        uint32_t total_dummy = 0; // not used—total across both cores
        UBaseType_t n =
            uxTaskGetSystemState(tasks.data(), tasks.size(), &total_dummy);
        tasks.resize(n);

        // Sum run-time counters per core, and remember IDLEx times
        uint64_t core_total[portNUM_PROCESSORS] = {0};
        uint64_t core_idle[portNUM_PROCESSORS] = {0};

        for (const auto &t : tasks) {
            int c = t.xCoreID; // 0 or 1 on ESP32
            if (c < 0 || c >= (int)portNUM_PROCESSORS)
                continue;

            core_total[c] += (uint64_t)t.ulRunTimeCounter;

            // Idle task names are "IDLE0" / "IDLE1"
            if ((c == 0 && strcmp(t.pcTaskName, "IDLE0") == 0) ||
                (c == 1 && strcmp(t.pcTaskName, "IDLE1") == 0)) {
                core_idle[c] = (uint64_t)t.ulRunTimeCounter;
            }
        }

        // Convert absolute counters into deltas across samples
        publish_from_delta(0, core_total[0], core_idle[0], this->core0);
        publish_from_delta(1, core_total[1], core_idle[1], this->core1);
    }

  private:
    struct Sample {
        uint64_t total = 0, idle = 0;
    };
    Sample prev_[portNUM_PROCESSORS] = {};

    void publish_from_delta(int core, uint64_t total_now, uint64_t idle_now,
                            esphome::Sensor *out) {
        uint64_t d_total = (total_now >= prev_[core].total)
                               ? (total_now - prev_[core].total)
                               : 0;
        uint64_t d_idle =
            (idle_now >= prev_[core].idle) ? (idle_now - prev_[core].idle) : 0;
        prev_[core].total = total_now;
        prev_[core].idle = idle_now;

        if (d_total == 0)
            return; // first sample or no change
        float util = 100.0f * (1.0f - (float)d_idle / (float)d_total);
        if (util < 0.f)
            util = 0.f;
        if (util > 100.f)
            util = 100.f;
        out->publish_state(std::round(util));
    }
};
