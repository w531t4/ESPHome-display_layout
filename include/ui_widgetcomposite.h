#pragma once
// #include <string>
#include <array>
#include "ui_shared.h"
#include "ui_widget.h"


template <std::size_t numWidgets>
class CompositeWidget: public Widget {
protected:
    std::array<std::unique_ptr<Widget>, numWidgets> members;
    ui::Coord anchor{-1, -1};
public:
    // Virtual destructor: mandatory in base classes with virtual functions
    virtual ~CompositeWidget() = default;


    // // Must perform initialization
    void initialize(const InitArgs& a) override {
        this->anchor = a.anchor;
    }

    void blank() {
        for (auto &ptr : members) {          // ptr is a std::unique_ptr<Widget>&
            if (ptr)                          // check it’s not empty
                ptr->blank();                  // call Widget’s member
        }
    }

    void write() {
        for (auto &ptr : members) {          // ptr is a std::unique_ptr<Widget>&
            if (ptr)                          // check it’s not empty
                ptr->write();                  // call Widget’s member
        }
    }

};
