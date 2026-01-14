// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "base_widget_dyntext.hpp"
#include "base_widget_text_string.hpp"

namespace ui {
// struct StringPostArgs {
//     std::string value;
// };

template <std::size_t BufSize>
class DynStringWidget
    : public DynTextWidget<std::string, StringPtrPostArgs, BufSize> {
  protected:
    constexpr const std::string default_fmt() { return std::string("%s"); }

    // Format into buf and update last
    virtual void prep(std::string value, const char *fmt) override {
        std::snprintf(this->buf.data(), this->buf.size(), fmt, value.c_str());
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
    };

  public:
};
} // namespace ui
