// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "display_layout.h"
#include "esphome/core/log.h"
#include <algorithm>
#include <cstdint>

#ifndef DISPLAY_LAYOUT_PROFILE
#define DISPLAY_LAYOUT_PROFILE 1
#endif

namespace esphome {
namespace display_layout {

constexpr int kWidgetKindCount = static_cast<int>(WidgetKind::PSN) + 1;

struct RenderProfile {
    uint32_t frames = 0;
    int64_t total_us = 0;
    int64_t post_us = 0;
    int64_t motion_us = 0;
    int64_t update_us = 0;
    int64_t relayout_us = 0;
    int64_t last_log_us = 0;
    int64_t post_kind_us[kWidgetKindCount]{};
    uint32_t post_kind_calls[kWidgetKindCount]{};
    int64_t weather_us = 0;
    uint32_t weather_calls = 0;
    int64_t weather_post_us = 0;
    int64_t weather_clock_us = 0;
    int64_t weather_state_us = 0;
    int64_t chat_us = 0;
    uint32_t chat_calls = 0;
    int64_t chat_history_us = 0;
    int64_t chat_post_us = 0;
    int64_t date_us = 0;
    uint32_t date_calls = 0;
    int64_t net_us = 0;
    uint32_t net_calls = 0;
    int64_t net_state_us = 0;
    int64_t net_post_us = 0;
    int64_t psn_us = 0;
    uint32_t psn_calls = 0;
    int64_t psn_pre_us = 0;
    int64_t psn_post_us = 0;
};

#if DISPLAY_LAYOUT_PROFILE
inline bool profile_should_log(RenderProfile &prof, int64_t now_us,
                               int64_t interval_us = 2000000) {
    if (prof.last_log_us == 0)
        prof.last_log_us = now_us;
    return (now_us - prof.last_log_us) >= interval_us;
}

template <typename KindToString>
inline void profile_log(RenderProfile &prof, const char *tag,
                        KindToString kind_to_string, int64_t now_us) {
    const float frames = static_cast<float>(prof.frames);
    const float inv = (frames > 0.0f) ? (1.0f / frames) : 1.0f;
    ESP_LOGI(tag,
             "render avg us: total=%.1f post=%.1f motion=%.1f update=%.1f "
             "relayout=%.1f over %.0f frames",
             prof.total_us * inv, prof.post_us * inv, prof.motion_us * inv,
             prof.update_us * inv, prof.relayout_us * inv, frames);

    struct PostEntry {
        int idx;
        float avg;
        float total;
        uint32_t calls;
    };
    PostEntry entries[kWidgetKindCount]{};
    for (int i = 0; i < kWidgetKindCount; ++i) {
        float avg = 0.0f;
        if (prof.post_kind_calls[i] != 0) {
            avg = static_cast<float>(prof.post_kind_us[i]) /
                  static_cast<float>(prof.post_kind_calls[i]);
        }
        entries[i] = PostEntry{
            i, avg, static_cast<float>(prof.post_kind_us[i]),
            prof.post_kind_calls[i]};
    }
    PostEntry entries_avg[kWidgetKindCount]{};
    PostEntry entries_total[kWidgetKindCount]{};
    std::copy(std::begin(entries), std::end(entries), std::begin(entries_avg));
    std::copy(std::begin(entries), std::end(entries),
              std::begin(entries_total));
    std::sort(std::begin(entries_avg), std::end(entries_avg),
              [](const PostEntry &a, const PostEntry &b) {
                  return a.avg > b.avg;
              });
    std::sort(std::begin(entries_total), std::end(entries_total),
              [](const PostEntry &a, const PostEntry &b) {
                  return a.total > b.total;
              });
    ESP_LOGI(tag,
             "post avg us (sorted 1/2): %s=%.1f %s=%.1f %s=%.1f %s=%.1f "
             "%s=%.1f",
             kind_to_string(static_cast<WidgetKind>(entries_avg[0].idx))
                 .c_str(),
             entries_avg[0].avg,
             kind_to_string(static_cast<WidgetKind>(entries_avg[1].idx))
                 .c_str(),
             entries_avg[1].avg,
             kind_to_string(static_cast<WidgetKind>(entries_avg[2].idx))
                 .c_str(),
             entries_avg[2].avg,
             kind_to_string(static_cast<WidgetKind>(entries_avg[3].idx))
                 .c_str(),
             entries_avg[3].avg,
             kind_to_string(static_cast<WidgetKind>(entries_avg[4].idx))
                 .c_str(),
             entries_avg[4].avg);
    ESP_LOGI(tag,
             "post avg us (sorted 2/2): %s=%.1f %s=%.1f %s=%.1f %s=%.1f "
             "%s=%.1f",
             kind_to_string(static_cast<WidgetKind>(entries_avg[5].idx))
                 .c_str(),
             entries_avg[5].avg,
             kind_to_string(static_cast<WidgetKind>(entries_avg[6].idx))
                 .c_str(),
             entries_avg[6].avg,
             kind_to_string(static_cast<WidgetKind>(entries_avg[7].idx))
                 .c_str(),
             entries_avg[7].avg,
             kind_to_string(static_cast<WidgetKind>(entries_avg[8].idx))
                 .c_str(),
             entries_avg[8].avg,
             kind_to_string(static_cast<WidgetKind>(entries_avg[9].idx))
                 .c_str(),
             entries_avg[9].avg);
    ESP_LOGI(
        tag,
        "post total us (sorted 1/2): %s=%.0f(%u) %s=%.0f(%u) %s=%.0f(%u) "
        "%s=%.0f(%u) %s=%.0f(%u)",
        kind_to_string(static_cast<WidgetKind>(entries_total[0].idx)).c_str(),
        entries_total[0].total, entries_total[0].calls,
        kind_to_string(static_cast<WidgetKind>(entries_total[1].idx)).c_str(),
        entries_total[1].total, entries_total[1].calls,
        kind_to_string(static_cast<WidgetKind>(entries_total[2].idx)).c_str(),
        entries_total[2].total, entries_total[2].calls,
        kind_to_string(static_cast<WidgetKind>(entries_total[3].idx)).c_str(),
        entries_total[3].total, entries_total[3].calls,
        kind_to_string(static_cast<WidgetKind>(entries_total[4].idx)).c_str(),
        entries_total[4].total, entries_total[4].calls);
    ESP_LOGI(
        tag,
        "post total us (sorted 2/2): %s=%.0f(%u) %s=%.0f(%u) %s=%.0f(%u) "
        "%s=%.0f(%u) %s=%.0f(%u)",
        kind_to_string(static_cast<WidgetKind>(entries_total[5].idx)).c_str(),
        entries_total[5].total, entries_total[5].calls,
        kind_to_string(static_cast<WidgetKind>(entries_total[6].idx)).c_str(),
        entries_total[6].total, entries_total[6].calls,
        kind_to_string(static_cast<WidgetKind>(entries_total[7].idx)).c_str(),
        entries_total[7].total, entries_total[7].calls,
        kind_to_string(static_cast<WidgetKind>(entries_total[8].idx)).c_str(),
        entries_total[8].total, entries_total[8].calls,
        kind_to_string(static_cast<WidgetKind>(entries_total[9].idx)).c_str(),
        entries_total[9].total, entries_total[9].calls);

    if (prof.weather_calls > 0 || prof.chat_calls > 0 || prof.date_calls > 0) {
        const float w_inv =
            (prof.weather_calls > 0)
                ? (1.0f / static_cast<float>(prof.weather_calls))
                : 0.0f;
        const float c_inv =
            (prof.chat_calls > 0)
                ? (1.0f / static_cast<float>(prof.chat_calls))
                : 0.0f;
        const float d_inv =
            (prof.date_calls > 0)
                ? (1.0f / static_cast<float>(prof.date_calls))
                : 0.0f;
        ESP_LOGI(tag,
                 "post detail avg us: weather=%.1f (clock=%.1f state=%.1f "
                 "post=%.1f) chat=%.1f (hist=%.1f post=%.1f) date=%.1f",
                 prof.weather_us * w_inv, prof.weather_clock_us * w_inv,
                 prof.weather_state_us * w_inv, prof.weather_post_us * w_inv,
                 prof.chat_us * c_inv, prof.chat_history_us * c_inv,
                 prof.chat_post_us * c_inv, prof.date_us * d_inv);
    }

    if (prof.net_calls > 0 || prof.psn_calls > 0) {
        const double interval_s =
            (now_us > prof.last_log_us)
                ? (static_cast<double>(now_us - prof.last_log_us) / 1e6)
                : 0.0;
        const float net_rate =
            (interval_s > 0.0)
                ? static_cast<float>(prof.net_calls / interval_s)
                : 0.0f;
        const float psn_rate =
            (interval_s > 0.0)
                ? static_cast<float>(prof.psn_calls / interval_s)
                : 0.0f;
        const float n_inv =
            (prof.net_calls > 0)
                ? (1.0f / static_cast<float>(prof.net_calls))
                : 0.0f;
        const float p_inv =
            (prof.psn_calls > 0)
                ? (1.0f / static_cast<float>(prof.psn_calls))
                : 0.0f;
        ESP_LOGI(tag,
                 "post detail avg us: net=%.1f (state=%.1f post=%.1f) "
                 "calls=%u rate=%.2f/s psn=%.1f (pre=%.1f post=%.1f) "
                 "calls=%u rate=%.2f/s",
                 prof.net_us * n_inv, prof.net_state_us * n_inv,
                 prof.net_post_us * n_inv, prof.net_calls, net_rate,
                 prof.psn_us * p_inv, prof.psn_pre_us * p_inv,
                 prof.psn_post_us * p_inv, prof.psn_calls, psn_rate);
    }

    prof = RenderProfile{};
    prof.last_log_us = now_us;
}
#else
inline bool profile_should_log(RenderProfile &, int64_t, int64_t = 0) {
    return false;
}

template <typename KindToString>
inline void profile_log(RenderProfile &, const char *, KindToString, int64_t) {
}
#endif

} // namespace display_layout
} // namespace esphome
