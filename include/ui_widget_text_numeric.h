#pragma once
#include "ui_shared.h"
#include "ui_widget.h"
#include "ui_widget_text.h"
#include <algorithm>
#include <iostream>

namespace ui {
template <typename T> struct NumericPostArgs {
    T value;
};

template <typename T, std::size_t BufSize>
class NumericWidget : public TextWidget<T, NumericPostArgs<T>, BufSize> {
  private:
    // Pick a default printf format based on T
    constexpr const std::string default_fmt() {
        if constexpr (std::is_integral<T>::value) {
            return std::string("%d");
        } else {
            return std::string("%0.f");
        }
    }

    // Format into buf and update last
    void prep(T value, const char *fmt) {
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
