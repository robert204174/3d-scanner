/*
MIT License

Copyright (c) 2020 Robert T. Adams

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef control_hpp_20200611_150851_PDT
#define control_hpp_20200611_150851_PDT

#include "stepper_control.hpp"
#include "rcode.hpp"


class Control
{
public:
  enum Mode { mode_normal, mode_test_limit_switches };
  using seek_count_t  = int16_t;

  Control();

  auto begin(Mode) -> void;

  auto run_command_processor() -> void;

private:
  using pin_value_t   = decltype(HIGH);
  using rcode_t       = RCode<Control, String>;
  enum class SeekOrientation { Forward, Reverse };

  StepperControl<Stepper, 200, 2, 3, 4, 5, 23> platform_;
  StepperControl<Stepper, 200, 6, 7, 8, 9, 25> carriage_; 

  static constexpr int limit_switch_min_     = 27;
  static constexpr int limit_switch_max_     = 29;

  struct Config 
  {
    int carriage_speed_       = 200;
    int carriage_seek_speed_  = 250;
    int carriage_seek_steps_  = 15;
    int platform_speed_       = 100;
    int carriage_max_         = 229;
  } config_;

  int carriage_position_ = 0;

  auto get_char() -> char;
  auto error(const String& msg) -> void;
  auto peek_char() -> int;

  auto get_line() -> String;
  auto reboot() -> void;
  auto halt() -> void;

  auto auto_set_max()  -> seek_count_t;
  auto auto_set_home() -> seek_count_t;
  auto limit_reached(int pin) -> bool;
  auto standby_all() -> void;
  auto resume_all() -> void;
  auto set_standby_all(pin_value_t) -> void;
  auto seek_limit(int limit_pin, SeekOrientation o) -> seek_count_t;
  
  auto rc_carriage_move_steps(const rcode_t&)     -> void;
  auto rc_carriage_set_home(const rcode_t&)       -> void;
  auto rc_carriage_set_span(const rcode_t&)       -> void;
  auto rc_platform_move_steps(const rcode_t& rc)  -> void;
  auto rc_reboot(const rcode_t& rc)               -> void;

  auto test_limit_switches() -> void;
};

#endif//control_hpp_20200611_150851_PDT
