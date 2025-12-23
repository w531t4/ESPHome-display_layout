// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#include "esphome/core/log.h"
#include "display_layout.h"

namespace esphome {
namespace display_layout {

static const char *TAG = "display_layout.component";

void DisplayLayout::setup() {

}

void DisplayLayout::loop() {

}

void DisplayLayout::dump_config(){
    ESP_LOGCONFIG(TAG, "Display Layout");
}


}  // namespace display_layout
}  // namespace esphome