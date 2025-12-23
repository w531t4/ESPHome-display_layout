// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <cstddef>

namespace ui {
struct IBufferResizable {
    virtual ~IBufferResizable() = default;
    virtual void set_capacity(std::size_t cap, bool preserve = true) = 0;
    virtual const size_t get_capacity() const = 0;
};
} // namespace ui
