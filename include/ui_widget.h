// MyBase.hpp
#pragma once
// #include <string>
#include "esphome/components/display/display.h"


class Widget {
public:
    // Virtual destructor: mandatory in base classes with virtual functions
    virtual ~Widget() = default;

    // ----- Mandatory functions for derived classes -----

    // Must return a name
    // virtual std::string getName() const = 0;

    // // Must perform initialization
    // virtual void initialize() = 0;

    // // // Must perform main work
    // // virtual void execute() = 0;

    // // check if blank is necessary
    // virtual bool is_different() = 0;

    // blank applicable space
    virtual void blank() = 0;

    // write-out to display
    virtual void write() = 0;
    // ----- Optional (can be overridden but not required) -----

    // // Provide default behavior
    // virtual void cleanup() { /* default no-op */ };

};
