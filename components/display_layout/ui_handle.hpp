// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "base_widget.hpp"

namespace ui {
template <class W> struct Handle {
    // Typed handle so callers can do handle->post(...)
    static_assert(std::is_base_of<Widget, W>::value,
                  "W must derive from Widget");
    W *ptr = nullptr;
    std::size_t index = npos;

    W *operator->() const { return ptr; }
    W &operator*() const { return *ptr; }
    explicit operator bool() const { return ptr != nullptr; }

    static constexpr std::size_t npos = static_cast<std::size_t>(-1);
};
} // namespace ui