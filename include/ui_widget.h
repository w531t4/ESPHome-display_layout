// MyBase.hpp
#pragma once
// #include <string>
#include <optional>
#include <any>
#include "esphome.h"
#include "esphome/components/font/font.h"
#include "esphome/components/display/display.h"
#include "ui_shared.h"

struct InitArgs {
    esphome::display::Display* it = nullptr;   // common
    ui::Coord anchor{0,0};                     // common
    esphome::font::Font* font = nullptr;       // often common

    // Optional commons (only some widgets care)
    std::optional<esphome::display::TextAlign> align;
    std::optional<esphome::Color> font_color;
    std::optional<esphome::Color> blank_color;
    std::optional<std::string> fmt;
    // Widget-specific payload (type-erased)
    std::any extras;
};
struct RunArgs {
    std::any extras;
};

class Widget {
public:
    // Virtual destructor: mandatory in base classes with virtual functions
    virtual ~Widget() = default;

    // ----- Mandatory functions for derived classes -----

    // Must return a name
    // virtual std::string getName() const = 0;

    // // Must perform initialization
    virtual void initialize(const InitArgs& args) = 0;;

    // // // Must perform main work
    // // virtual void execute() = 0;

    // // check if blank is necessary
    // virtual bool is_different() = 0;

    // blank applicable space
    virtual void blank() = 0;

    // write-out to display
    virtual void write() = 0;

    virtual void run(const RunArgs& args) = 0;
    // ----- Optional (can be overridden but not required) -----

    // // Provide default behavior
    // virtual void cleanup() { /* default no-op */ };

};
