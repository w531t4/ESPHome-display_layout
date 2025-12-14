#pragma once
#include "ui_shared.hpp"
#include "ui_widget.hpp"
#include "ui_widget_dyntext.hpp"
#include <iostream>

namespace ui {
// struct StringPostArgs {
//     std::string value;
// };

template <std::size_t BufSize>
class DynStringWidget
    : public DynTextWidget<std::string, StringPostArgs, BufSize> {
  protected:
    constexpr const std::string default_fmt() { return std::string("%s"); }

    // Format into buf and update last
    virtual void prep(std::string value, const char *fmt) override {
        std::snprintf(this->buf.data(), this->buf.size(), fmt, value.c_str());
        this->last = std::move(value);
    }

  public:
};
} // namespace ui
