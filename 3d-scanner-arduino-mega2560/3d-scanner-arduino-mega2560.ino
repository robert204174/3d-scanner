#include "control.hpp"

#include <Arduino.h>

auto control = Control();

void setup() {
  control.begin(Control::mode_normal);
}

void loop() {
  control.run_command_processor();
}
