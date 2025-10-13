// ui_capabilities.h
#pragma once
#include <cstddef>

namespace ui {
    struct IBufferResizable {
        virtual ~IBufferResizable() = default;
        virtual void set_capacity(std::size_t cap, bool preserve = true) = 0;
    };
} // namespace ui

