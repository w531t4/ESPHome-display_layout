// MyBase.hpp
#pragma once
// #include <string>
#include "argsbag.h"
#include "esphome.h"
#include "esphome/components/display/display.h"
#include "esphome/components/font/font.h"
#include "magnet.h"
#include "ui_logging.h"
#include "ui_shared.h"
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
    bool enabled = true;
    bool initialized = false;
    ui::Coord anchor{-1, -1};
    Magnet magnet;
    std::string id;

  public:
    // Virtual destructor: mandatory in base classes with virtual functions
    virtual ~Widget() = default;
    bool is_enabled() const noexcept { return enabled; }
    void set_enabled(const bool state) { this->enabled = state; }
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
    const std::string get_name() { return this->id; }

    // blank applicable space
    virtual void blank() = 0;

    // write-out to display
    virtual void write() = 0;

    // use to push value into the widget, but don't process until update()
    virtual void post(const PostArgs &args) = 0;

    virtual void update() = 0;
    virtual void horizontal_shift(const int pixels) {
        ui::log_horizontal_shift(this->id, this->anchor.x,
                                 this->anchor.x + pixels);
        this->anchor.x = this->anchor.x + pixels;
    }
    const Magnet get_magnet() { return this->magnet; }
    ui::Coord anchor_value() const noexcept {
        return anchor;
    } // non-virtual is fine if stored in base
    virtual const int width() = 0;
    virtual const int height() = 0;
    // ----- Optional (can be overridden but not required) -----

    // // Provide default behavior
    // virtual void cleanup() { /* default no-op */ };
};
