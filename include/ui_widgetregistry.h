// ui_widgetregistry.h
#pragma once
#include <array>
#include <cstddef>
#include <type_traits>
#include "ui_widget.h"
#include "ui_shared.h"
#include <algorithm>
#include <limits>

namespace ui {

  template <std::size_t MaxWidgets>
  class WidgetRegistry {
  private:
    std::array<Widget*, MaxWidgets> items_{};  // non-owning raw pointers
    std::size_t count_ = 0;
    int gap_x_ = 0;
    int right_edge_base_ = std::numeric_limits<int>::min(); // unset sentinel
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
    void relayout() {
      if (count_ == 0) return;
      // collect enabled items
      Widget* active[MaxWidgets];
      std::size_t n = 0;
      for (std::size_t i = 0; i < count_; ++i)
        if (items_[i] && items_[i]->is_enabled())
          active[n++] = items_[i];
      if (n == 0) return;

      // sort by priority (lower value = higher priority)
      std::sort(active, active + n,
                [](const Widget* a, const Widget* b) {
                  return a->get_priority() < b->get_priority();
                });

      // choose right edge: use configured base if set, else infer from current layout
      int right_edge;
      if (right_edge_base_ != std::numeric_limits<int>::min()) {
        right_edge = right_edge_base_;
      } else {
        right_edge = std::numeric_limits<int>::min();
        for (std::size_t i = 0; i < n; ++i) {
          const ui::Coord a = active[i]->anchor_value();
          const int r = a.x + active[i]->width();
          if (r > right_edge) right_edge = r;
        }
      }

      // total width including gaps
      int total = 0;
      for (std::size_t i = 0; i < n; ++i) total += active[i]->width();
      total += (n > 0 ? (static_cast<int>(n) - 1) * gap_x_ : 0);

      int x = right_edge;
      for (std::size_t i = 0; i < n; ++i) {
        Widget* w = active[i];
        const int wpx = w->width();
        const int target_x = x - wpx;             // place so its right edge hits current x
        const int cur_x = w->anchor_value().x;
        const int dx = target_x - cur_x;
        if (dx != 0) w->horizontal_shift(dx);     // updates anchor & redraws
        x = target_x;                             // move left for next widget
        if (i + 1 < n) x -= gap_x_;
      }
    }

    void set_right_edge_x(int px) { right_edge_base_ = px; }
    void set_gap_x(int px) { gap_x_ = px; }
    void set_right_anchored(bool) {}

  };

} // namespace ui
