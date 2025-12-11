#pragma once

#include "esphome/core/log.h"

namespace ui {
    static const char *const MYTAG = "ecs";
    void log_horizontal_shift(const std::string &name, const int prev_x, const int x) {
        char buf[256];

        int len = snprintf(
            buf,
            sizeof(buf),
            // JSON template
            "{\"event\":{\"kind\":\"state\",\"type\":[\"change\"],\"action\":\"horizontal_shift\"},"
            "\"myobj\":{\"name\":\"%s\",\"position\":{\"x_prev\":%d,\"x\":%d}}}",
            name.c_str(),
            prev_x,
            x
        );

        if (len < 0 || len >= (int)sizeof(buf)) {
            // Truncated or error — at least log something sane
            ESP_LOGW(ui::MYTAG, "horizontal_shift log truncated (name=%s prev_x=%d x=%d)",
                    name.c_str(), prev_x, x);
            return;
        }

        // Correct: format string + %s
        ESP_LOGD(ui::MYTAG, "%s", buf);
    }

}