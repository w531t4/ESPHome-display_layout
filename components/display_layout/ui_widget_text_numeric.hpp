// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "ui_widget_text.hpp"

namespace ui {
template <typename T> struct NumericPostArgs {
    T value;
};

template <typename T, std::size_t BufSize>
class NumericWidget : public TextWidget<T, NumericPostArgs<T>, BufSize> {
  private:
    // Pick a default printf format based on T
    constexpr const std::string default_fmt() const override {
        if constexpr (std::is_integral<T>::value) {
            return std::string("%d");
        } else {
            return std::string("%0.f");
        }
    }

    // Format into buf
    void prep(T value, const char *fmt) override {
        if constexpr (std::is_integral<T>::value) {
            // std::snprintf(buf, sizeof(buf), fmt, static_cast<long
            // long>(value));
            std::snprintf(this->buf, sizeof(this->buf), fmt, value);
        } else {
            std::snprintf(this->buf, sizeof(this->buf), fmt,
                          static_cast<double>(value));
        }
        this->last = value;
    }

  public:
};
} // namespace ui
