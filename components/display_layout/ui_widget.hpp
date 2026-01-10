// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once
#include "argsbag.hpp"
#include "esphome/components/display/display.h"
#include "esphome/components/font/font.h"
#include "magnet.hpp"
#include "ui_logging.hpp"
#include "ui_shared.hpp"
#include <any>
#include <optional>

struct InitArgs {
    esphome::display::Display *it = nullptr; // common
    std::string id;
    ui::Coord anchor{0, 0}; // common
    uint8_t priority = 0;
    // Optional commons (only some widgets care)
    std::optional<esphome::font::Font *> font = nullptr;
    std::optional<esphome::display::TextAlign> align;
    std::optional<esphome::Color> font_color;
    std::optional<esphome::Color> blank_color;
    std::optional<std::string> fmt;
    std::optional<esphome::font::Font *> font2;
    std::optional<esphome::Color> font2_color;
    std::optional<Magnet> magnet;
    // Widget-specific payload (type-erased)
    ArgsBag extras;
};
struct PostArgs {
    std::any extras;
};

class Widget {
  protected:
    esphome::display::Display *it = nullptr;
    uint8_t priority = 0;

    // Togglable to free up any/all cycles for working with/on a widget.
    bool enabled = true;

    // Togglable to hide a component from the display. A hidden widget must
    // occupy no space on the resultant display
    bool visible = true;
    bool initialized = false;

    // Originally, post() acted as a facilitator of data storage, and update()
    // incorporated logic for testing whether there was a change to gate writing pixels
    // unnecessarily

    // 'dirty' intends to short-circuit that a bit... post() will be responsible for
    // determining if a change has occurred (using in-flight pointers, etc). If a change,
    // it stores the value and sets dirty. update() will be responsible for measuring
    // dirty and clearing it.
    bool dirty = false;
    ui::Coord anchor{-1, -1};
    Magnet magnet;
    std::string id;

  public:
    // Virtual destructor: mandatory in base classes with virtual functions
    virtual ~Widget() = default;
    virtual const bool is_dirty() const noexcept { return dirty; }
    void set_dirty(const bool state) { this->dirty = state; }
    bool is_enabled() const noexcept { return enabled; }
    void set_enabled(const bool state) { this->enabled = state; }
    bool is_visible() const noexcept { return visible; }
    void set_visible(const bool state) { this->visible = state; }
    uint8_t get_priority() const noexcept { return priority; }
    // ----- Mandatory functions for derived classes -----

    // Must return a name
    // virtual std::string getName() const = 0;

    // // Must perform initialization
    virtual void initialize(const InitArgs &args) {
        this->it = args.it;
        this->id = args.id;
        this->anchor = args.anchor;
        this->priority = args.priority;
        this->magnet = args.magnet.value_or(Magnet::RIGHT);
    }

    // // // Must perform main work
    // // virtual void execute() = 0;

    // // check if blank is necessary
    // virtual bool is_different() = 0;

    virtual void draw_outline(const esphome::Color &color) {
        this->it->rectangle(this->anchor.x, this->anchor.y, this->width(),
                            this->height(), color);
    }
    const std::string get_name() const { return this->id; }

    // blank applicable space
    //  - widget must be enabled, and visible.
    virtual void blank() = 0;

    // write content to the display.
    //  - must never run if widget is _hidden_ or _disabled_
    virtual void write() = 0;

    // use to push value into the widget, but don't process until update()
    virtual void post(const PostArgs &args) = 0;

    virtual void update() = 0;
    virtual void horizontal_shift(const int pixels) {
        ui::log_horizontal_shift(this->id, this->anchor.x,
                                 this->anchor.x + pixels);
        this->anchor.x = this->anchor.x + pixels;
    }
    const Magnet get_magnet() const { return this->magnet; }
    ui::Coord anchor_value() const noexcept {
        return anchor;
    } // non-virtual is fine if stored in base
    virtual const int width() const = 0;
    virtual const int height() const = 0;
    // ----- Optional (can be overridden but not required) -----

    // // Provide default behavior
    // virtual void cleanup() { /* default no-op */ };
    const int get_left_edge_x() const {
        // *--------------|
        // |              |
        // |--------------|
        //
        // #<---- returned value

        return this->anchor_value().x;
    }

    const int get_right_edge_x() const {
        // *--------------|
        // |              |
        // |--------------|
        //
        // x<-----width-->#<---- returned value

        return this->anchor_value().x + this->width();
    }
};
