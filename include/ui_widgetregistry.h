// ui_widgetregistry.h
#pragma once
#include <array>
#include <cstddef>
#include <type_traits>
#include "ui_widget.h"
#include "ui_shared.h"

namespace ui {

  template <std::size_t MaxWidgets>
  class WidgetRegistry {
  private:
    std::array<Widget*, MaxWidgets> items_{};  // non-owning raw pointers
    std::size_t count_ = 0;
  public:
    // Typed handle so callers can do handle->post(...)
    template <class W>
    struct Handle {
      static_assert(std::is_base_of<Widget, W>::value, "W must derive from Widget");
      W* ptr = nullptr;
      std::size_t index = npos;

      W* operator->() const { return ptr; }
      W& operator*()  const { return *ptr; }
      explicit operator bool() const { return ptr != nullptr; }

      static constexpr std::size_t npos = static_cast<std::size_t>(-1);
    };

    WidgetRegistry() = default;

    // ----- Phase 1: registration (startup only, no heap) -----
    template <class W>
    Handle<W> add(W& w) {
      static_assert(std::is_base_of<Widget, W>::value, "W must derive from Widget");
      if (count_ >= MaxWidgets) return {};   // invalid handle
      items_[count_] = &w;                   // store as base pointer (non-owning)
      Handle<W> h; h.ptr = &w; h.index = count_;
      ++count_;
      return h;
    }

    std::size_t size() const noexcept { return count_; }
    Widget* at(std::size_t i) noexcept { return (i < count_) ? items_[i] : nullptr; }
    const Widget* at(std::size_t i) const noexcept { return (i < count_) ? items_[i] : nullptr; }

    // ----- Phase 2 fan-out (no timing) -----
    void update_all() {
      for (std::size_t i = 0; i < count_; ++i)
        if (items_[i] && items_[i]->is_enabled())
          items_[i]->update();
    }

    void post_all(const PostArgs& args) {
      for (std::size_t i = 0; i < count_; ++i)
        if (items_[i] && items_[i]->is_enabled())
          items_[i]->post(args);
    }

    void blank_all() {
      for (std::size_t i = 0; i < count_; ++i)
        if (items_[i] && items_[i]->is_enabled())
          items_[i]->blank();
    }

    void write_all() {
      for (std::size_t i = 0; i < count_; ++i)
        if (items_[i] && items_[i]->is_enabled())
          items_[i]->write();
    }

    // Placeholders for later phases (no-ops now)
    void relayout() {}
    void set_gap_x(int) {}
    void set_right_anchored(bool) {}

  };

} // namespace ui
