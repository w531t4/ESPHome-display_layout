
// SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
// SPDX-License-Identifier: MIT
#pragma once

#include "esphome/core/component.h"
#include "argsbag.hpp"
namespace esphome {
namespace display_layout {

class DisplayLayout : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
};


}  // namespace display_layout
}  // namespace esphome