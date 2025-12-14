// ui_widgetregistry.h
#pragma once
#include "magnet.hpp"
#include "ui_shared.hpp"
#include "ui_widget.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace ui {

// ---- add this tiny trait at file scope ---------------------------------
// Detects presence of set_capacity(std::size_t, bool) on W
template <class T, class = void> struct has_set_capacity : std::false_type {};
template <class T>
struct has_set_capacity<
    T, std::void_t<decltype(std::declval<T &>().set_capacity(
           std::declval<std::size_t>(), std::declval<bool>()))>>
    : std::true_type {};

// Detects presence of size_t capacity() const on W
template <class T, class = void> struct has_get_capacity : std::false_type {};
template <class T>
struct has_get_capacity<
    T, std::void_t<decltype(std::declval<const T &>().get_capacity())>>
    : std::true_type {};

template <std::size_t MaxWidgets> class WidgetRegistry {
  private:
    struct Entry {
        Widget *ptr = nullptr;
        void (*set_capacity)(Widget *, std::size_t,
                             bool) = nullptr; // null if unsupported
        std::size_t (*get_capacity)(const Widget *) = nullptr; // ← add this
    };
    std::array<Entry, MaxWidgets> items_{}; // non-owning
    std::size_t count_ = 0;
    int gap_x_ = 0; // Number of pixels in-between widgets
    int right_edge_base_ =
        std::numeric_limits<int>::min(); // All right-aligned widgets will be
                                         // placed to the immediate left of this
                                         // position
    int left_edge_base_ = 0; // All left-aligned widgets will be placed to the
                             // immediate right of this position
  public:
    // Typed handle so callers can do handle->post(...)
    template <class W> struct Handle {
        static_assert(std::is_base_of<Widget, W>::value,
                      "W must derive from Widget");
        W *ptr = nullptr;
        std::size_t index = npos;

        W *operator->() const { return ptr; }
        W &operator*() const { return *ptr; }
        explicit operator bool() const { return ptr != nullptr; }

        static constexpr std::size_t npos = static_cast<std::size_t>(-1);
    };

    WidgetRegistry() = default;

    // ----- Phase 1: registration (startup only, no heap) -----
    template <class W> Handle<W> add(W &w) {
        static_assert(std::is_base_of<Widget, W>::value,
                      "W must derive from Widget");
        if (count_ >= MaxWidgets)
            return {}; // invalid handle

        Entry &e = this->items_[count_];
        e.ptr = &w;

        // If W has set_capacity(std::size_t,bool), remember how to call it
        if constexpr (has_set_capacity<W>::value) {
            e.set_capacity = +[](Widget *base, std::size_t cap, bool preserve) {
                static_cast<W *>(base)->set_capacity(cap, preserve);
            };
        } else {
            e.set_capacity = nullptr;
        }

        if constexpr (has_get_capacity<W>::value) {
            e.get_capacity = +[](const Widget *base) -> std::size_t {
                return static_cast<const W *>(base)->get_capacity();
            };
        } else {
            e.get_capacity = nullptr;
        }
        Handle<W> h;
        h.ptr = &w;
        h.index = count_;
        ++count_;
        return h;
    }

    std::size_t size() const noexcept { return count_; }
    Widget *at(std::size_t i) noexcept {
        return (i < count_) ? items_[i].ptr : nullptr;
    }
    const Widget *at(std::size_t i) const noexcept {
        return (i < count_) ? items_[i].ptr : nullptr;
    }

    // ----- Phase 2 fan-out (no timing) -----
    void update_all() {
        for (std::size_t i = 0; i < count_; ++i)
            if (at(i) && at(i)->is_enabled())
                at(i)->update();
    }

    void post_all(const PostArgs &args) {
        for (std::size_t i = 0; i < count_; ++i)
            if (at(i) && at(i)->is_enabled())
                at(i)->post(args);
    }

    void blank_all() {
        for (std::size_t i = 0; i < count_; ++i)
            if (at(i) && at(i)->is_enabled())
                at(i)->blank();
    }

    void write_all() {
        for (std::size_t i = 0; i < count_; ++i)
            if (at(i) && at(i)->is_enabled())
                at(i)->write();
    }

    void relayout(int size = -1) {
        int last_pos = -1, left = -1, right = -1;
        bool redraw_needed = false;
        relayout_left(last_pos, redraw_needed);
        if (last_pos != -1)
            left = last_pos + gap_x_;
        last_pos = -1;
        relayout_right(last_pos, redraw_needed);
        if (last_pos != -1)
            right = last_pos;
        int delta = right - left;
        if (delta > 0 && (right >= 0) && (left >= 0)) {
            relayout_auto(left, size > 0 ? size : delta, redraw_needed);
        }
        if (redraw_needed) {
            this->blank_all();
            this->write_all();
        }
    }

    void relayout_auto(const int edge_anchor, const int new_capacity,
                       bool &redraw_needed) {
        if (count_ == 0)
            return;
        for (std::size_t i = 0; i < count_; ++i) {
            Entry &e = items_[i];
            auto *w = e.ptr;
            if (!w || !w->is_enabled() || w->get_magnet() != Magnet::AUTO)
                continue;

            if (!(e.set_capacity && e.get_capacity))
                return;

            if (e.get_capacity(w) != new_capacity) {
                ESP_LOGW("registry",
                         "setting new dynamic widget capacity to cap=%d, "
                         "current=%d",
                         new_capacity, e.get_capacity(w));
                e.set_capacity(w, new_capacity, true);
            }

            const int cur_x = w->anchor_value().x;
            if (cur_x != edge_anchor) {
                ESP_LOGW("registry", "performing shift val=%d",
                         edge_anchor - cur_x);
                w->blank();
                w->horizontal_shift(edge_anchor - cur_x);
                redraw_needed = true;
            }
            return;
        }
    }

    std::size_t get_enabled_and_oriented_widgets(Widget **w_array,
                                                 Magnet orientation) {
        // Create array of Widget pointers
        // meeting the following critera:
        //    - widget is enabled
        //    - widget matches desired orientation
        // return number of qualifying items
        std::size_t n = 0;
        for (std::size_t i = 0; i < count_; ++i) {
            if (at(i) && at(i)->is_enabled() &&
                at(i)->get_magnet() == orientation)
                w_array[n++] = at(i);
        }
        return n;
    }

    const int get_right_edge(Widget *item) {
        // *--------------|
        // |              |
        // |--------------|
        //
        // x<-----width-->#<---- returned value

        return item->anchor_value().x + item->width();
    }

    const int get_leftmost_edge(Widget **items, const size_t max_items) {
        int edge = 0;
        if (left_edge_base_ < 0) {
            edge = left_edge_base_;
        } else {
            for (std::size_t i = 0; i < max_items; ++i) {
                // it's odd, but the logic is the same...
                const int l = get_right_edge(items[i]);
                if (l < edge)
                    edge = l;
            }
        }
        return edge;
    }

    void relayout_left(int &last_pos, bool &redraw_needed) {
        // For now, this is for twitch streamer icons
        // **************************
        // i suspect we're not actually finding the leftmost edge... but more of
        // the edge important to magnet::LEFT
        // ******************************
        if (count_ == 0)
            return;
        // collect enabled items
        Widget *active[MaxWidgets];
        std::size_t n = get_enabled_and_oriented_widgets(active, Magnet::LEFT);
        if (n == 0)
            return;

        std::sort(active, active + n, [](const Widget *a, const Widget *b) {
            return a->get_priority() < b->get_priority();
        });

        int left_edge = get_leftmost_edge(active, n);
        int x = left_edge;
        for (std::size_t i = 0; i < n; ++i) {
            Widget *w = active[i];
            const int next_x = x + w->width();
            const int cur_x = w->anchor_value().x;
            if (cur_x < x) {
                w->blank();
                w->horizontal_shift(x - cur_x);
                redraw_needed = true;
            }
            x = next_x;
            if (i + 1 < n)
                x += gap_x_;
        }
        last_pos = x;
    }

    void relayout_right(int &last_pos, bool &redraw_needed) {
        if (count_ == 0)
            return;
        Widget *active[MaxWidgets];
        std::size_t n = get_enabled_and_oriented_widgets(active, Magnet::RIGHT);
        if (n == 0)
            return;

        std::sort(active, active + n, [](const Widget *a, const Widget *b) {
            return a->get_priority() < b->get_priority();
        });

        int right_edge = std::numeric_limits<int>::min();
        if (right_edge_base_ != std::numeric_limits<int>::min()) {
            right_edge = right_edge_base_;
        } else {
            for (std::size_t i = 0; i < n; ++i) {
                const int r = get_right_edge(active[i]);
                if (r > right_edge)
                    right_edge = r;
            }
        }

        int x = right_edge;
        for (std::size_t i = 0; i < n; ++i) {
            Widget *w = active[i];
            const int wpx = w->width();
            const int target_x = x - wpx;
            const int cur_x = w->anchor_value().x;
            const int dx = target_x - cur_x;
            if (dx != 0) {
                w->blank();
                w->horizontal_shift(dx);
                redraw_needed = true;
            }
            x = target_x;
            if (i + 1 < n)
                x -= gap_x_;
        }
        last_pos = x;
    }

    void set_right_edge_x(int px) { right_edge_base_ = px; }
    void set_gap_x(int px) { gap_x_ = px; }
    void set_right_anchored(bool) {}
};

} // namespace ui
