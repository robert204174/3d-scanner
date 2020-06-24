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

#include "Adafruit_VL53L0X.h"
#include "fake_pair.hpp"
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
  using rcode_t       = RCode<String>;
  enum class SeekOrientation { Forward, Reverse };

  StepperControl<Stepper, 200, 2, 3, 4, 5, 23> platform_;
  StepperControl<Stepper, 200, 6, 7, 8, 9, 25> carriage_; 

  Adafruit_VL53L0X tof_sensor_;

  static constexpr int limit_switch_min_     = 27;
  static constexpr int limit_switch_max_     = 29;

  struct Config 
  {
    int carriage_speed_       = 200;
    int carriage_seek_speed_  = 100;
    int carriage_seek_steps_  = 15;
    int platform_speed_       = 100;
    int carriage_max_         = 229;
  } config_;

  int carriage_position_ = 0;

  // text processing;
  // probably should be moved out to another class, but whatev, it's easier to 
  // halt on errors this way (and Arduino is hamstrung w/o stdlib)
  auto data_to_bool(const String& d) -> pair<bool, bool>;
  auto data_to_int(const String& d) -> pair<bool, int>;
  auto get_char() -> char;
  auto get_line() -> String;
  auto is_valid_for_int(int ch) -> bool;
  auto peek_char() -> int;
  // errors and logging
  auto error(const String& msg)                   -> void;
  auto error_expected_bool(const String& data)    -> void; 
  auto error_expected_int(const String& data)     -> void;

  auto auto_set_max()  -> seek_count_t;
  auto auto_set_home() -> seek_count_t;

  auto halt() -> void;
  auto limit_reached(int pin) -> bool;
  auto reboot() -> void;
  auto resume_all() -> void;
  auto seek_limit(int limit_pin, SeekOrientation o) -> seek_count_t;
  auto set_standby_all(pin_value_t) -> void;
  auto standby_all() -> void;

  // rc functions
  auto rc_carriage_move_steps(const rcode_t&)     -> void;
  auto rc_carriage_set_home(const rcode_t&)       -> void;
  auto rc_carriage_set_span(const rcode_t&)       -> void;
  auto rc_log_info(const rcode_t& rc)             -> void;
  auto rc_platform_move_steps(const rcode_t& rc)  -> void;
  auto rc_platform_speed(const rcode_t& rc)       -> void;
  auto rc_rangefinder_ping(const rcode_t&)        -> void;
  // rc system functions
  auto rc_reboot(const rcode_t& rc)               -> void;
  auto rc_system_poll(const rcode_t& rc)               -> void;

  // test functions
  auto test_limit_switches() -> void;

  // function to automatically enable/disable stepper before/after movement,
  // respectively
template<typename StepperT, typename FnT>
  auto auto_standby(StepperT& stepper, FnT&& in_between_fn) -> void
    {
      stepper.resume();
      in_between_fn();
      stepper.standby();
    }

  // structure for mapping rcodes to member functions
  struct map_entry
  {
    using callback_t = auto(Control::*)(const rcode_t&) -> void;
    const char* name;
    callback_t  callback;
  };
  // array of rcode mapping
  static auto rcode_map() -> const auto&
    {
      static map_entry rm[] =
        {
          map_entry { "carriage.move.steps"     , &Control::rc_carriage_move_steps  },
          map_entry { "carriage.auto_set_home"  , &Control::rc_carriage_set_home    },
          map_entry { "carriage.auto_set_span"  , &Control::rc_carriage_set_span    },
          map_entry { "log.debug"               , &Control::rc_log_info             },
          map_entry { "log.error"               , &Control::rc_log_info             },
          map_entry { "log.info"                , &Control::rc_log_info             },
          map_entry { "log.warning"             , &Control::rc_log_info             },
          map_entry { "platform.move.steps"     , &Control::rc_platform_move_steps  },
          map_entry { "platform.speed"          , &Control::rc_platform_speed       },
          map_entry { "rangefinder.ping"        , &Control::rc_rangefinder_ping     },
          map_entry { "reboot"                  , &Control::rc_reboot               },
          map_entry { "system.poll"             , &Control::rc_system_poll          },
          map_entry { nullptr                   , nullptr                           }
        };
      return rm;
    }
template<typename CallbackT>
  static auto for_each_function(CallbackT&& callback)
    {
      for(size_t i = 0; rcode_map()[i].name != nullptr; ++i)
      {
        callback(rcode_map()[i]);
      }
    }
};

#endif//control_hpp_20200611_150851_PDT
