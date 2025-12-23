// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "ui_widget_text.hpp"

namespace ui {
struct StringPostArgs {
    std::string value;
};

template <std::size_t BufSize>
class StringWidget : public TextWidget<std::string, StringPostArgs, BufSize> {
  private:
    constexpr const std::string default_fmt() { return std::string("%s"); }

    // Format into buf and update last
    void prep(std::string value, const char *fmt) {
        std::snprintf(this->buf, sizeof(this->buf), fmt, value.c_str());
        this->last = value;
    }

  public:
};
} // namespace ui
