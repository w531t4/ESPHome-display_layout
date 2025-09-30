#pragma once
// #include <string>
#include <array>
#include "ui_shared.h"
#include "ui_widget.h"


template <std::size_t numWidgets>
class CompositeWidget: public Widget {
protected:
    std::array<std::unique_ptr<Widget>, numWidgets> members;
public:
    // Virtual destructor: mandatory in base classes with virtual functions
    virtual ~CompositeWidget() = default;


    // // Must perform initialization
    void initialize(const InitArgs& a) override {
        Widget::initialize(a);
        initialized = true;
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

    void update() {
        for (auto &ptr : members) {
            if (ptr) {
                ptr->update();
            }
        }
    }

    void horizontal_shift(const int pixels) {
        this->anchor.x = this->anchor.x + pixels;
        for (auto &ptr : members) {
            if (ptr) {
                ptr->horizontal_shift(pixels);
            }
        }
    }

    const int width() override {
        bool found = false;
        int min_x = std::numeric_limits<int>::max();
        int max_right = std::numeric_limits<int>::min();

        for (const auto& p : members) {
            if (!p) continue;
            int x = p->anchor_value().x;
            int right = x + p->width();   // child’s right edge
            if (!found || x < min_x)      min_x = x;
            if (!found || right > max_right) max_right = right;
            found = true;
        }

        return found ? (max_right - min_x) : 0;
    }

};
