// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "base_widget_text.hpp"

namespace ui {
struct StringPostArgs {
    std::string value;
};

struct StringPtrPostArgs {
    const std::string *ptr;
};

template <std::size_t BufSize>
class StringWidget
    : public TextWidget<std::string, StringPtrPostArgs, BufSize> {
  private:
    constexpr const std::string default_fmt() const override {
        return std::string("%s");
    }

    // Format into buf
    void prep(std::string value, const char *fmt) override {
        std::snprintf(this->buf, sizeof(this->buf), fmt, value.c_str());
    }
    bool is_different(StringPtrPostArgs value) const override {
        if (!this->last.has_value())
            return true;
        if (value.ptr == nullptr)
            return false;
        return *value.ptr != this->last.value();
    }
    void copy_value(StringPtrPostArgs value) override {
        if (value.ptr == nullptr)
            return;
        this->last = *value.ptr;
    }

  public:
};
} // namespace ui
