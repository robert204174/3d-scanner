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
#include "control.hpp"
#include "logger.hpp"
#include "rcode.hpp"
#include <avr/wdt.h>
#include <Arduino.h>

namespace 
  {
    constexpr int   reboot_delay_             = 1000;
    constexpr char  param_prefix_             = '\\';
    constexpr char  command_prefix_           = '@';
    constexpr char  integer_prefix_           = param_prefix_;
    constexpr int   screw_pitch_mm_           = 2;
    constexpr int   screw_starts_             = 4;
    constexpr int   screw_max_height_mm_      = 200;
    constexpr auto  screw_travel_per_turn_mm_ = screw_pitch_mm_ * screw_starts_;
    constexpr auto  screw_max_turns_          = screw_max_height_mm_ / screw_travel_per_turn_mm_;
  }
Control::Control()
: platform_()
, carriage_()
  {
  }
auto Control::begin(Mode m) -> void
  {
    platform_.begin();
    platform_.set_speed(config_.platform_speed_);
    carriage_.begin();
    carriage_.set_speed(config_.carriage_speed_);
    // initialize I/O pins
    pinMode(limit_switch_min_,  INPUT_PULLUP);
    pinMode(limit_switch_max_,  INPUT_PULLUP);
    // setup serial
    Serial.begin(115200);
    while(!Serial) { yield(); }
    // setup TOF sensor
    constexpr bool failure = false;
    // test for failure conditions
    if(tof_sensor_.begin() == failure)
    {
      Log::error()("Time-of-flight sensor initialization failed.");
      halt();
    }
    // check mode
    if(m == mode_normal)
    {
      abs(auto_set_home());
      Serial.println("#ready");
    }
    else // test and debug modes (will halt when finished)
    {
      if(m == mode_test_limit_switches)
      {
        test_limit_switches();
      }
      else  
      {
        Log::error()("Invalid mode requested");
      }
      halt();
    }
  }
auto Control::reboot() -> void
  {
    Log::info()("Rebooting...");
    delay(reboot_delay_);
    // start watchdog timer
    wdt_enable(WDTO_15MS);
    // let the timer expire
    while(true) {}
  }
auto Control::halt() -> void
  {
    Serial.println("#break");
    standby_all();
    Serial.println("#terminate");
    Log::info()("Type @reboot to reboot device");
    while(true) 
    { 
      auto cmd = get_line();
      if(cmd == "@reboot")
      {
        reboot();
        return;
      }
      yield(); 
    }
  }
auto Control::limit_reached(int limit_pin) -> bool
  {
    return digitalRead(limit_pin) == LOW;
  }
auto Control::seek_limit(int limit_pin, SeekOrientation o) -> seek_count_t 
  {
    int count = 0;
    int orientation = o == SeekOrientation::Forward? 1 : -1;
    int seek_steps = orientation * config_.carriage_seek_steps_;
    int back_steps = -orientation;
    carriage_.resume();
    carriage_.set_speed(config_.carriage_seek_speed_);
    while(limit_reached(limit_pin) == false)
    {
      count += seek_steps;
      carriage_.step(seek_steps);
    }
    while(limit_reached(limit_pin) == true)
    {
      count += back_steps;
      carriage_.step(back_steps);
    }
    carriage_.standby();
    return count;
  }
auto Control::auto_set_max() -> seek_count_t
  {
    Log::info()("Searching for home...");
    auto count = seek_limit(limit_switch_max_, SeekOrientation::Forward);
    Log::info()("Homing complete.");
    config_.carriage_max_ = count;
    return count;
  }
auto Control::auto_set_home() -> seek_count_t
  {
    Log::info()("Searching for home...");
    auto count = seek_limit(limit_switch_min_, SeekOrientation::Reverse);
    Log::info()("Homing complete.");
    carriage_position_ = 0;
    return count;
  }
auto Control::resume_all() -> void
  {
    set_standby_all(HIGH);
  }
auto Control::standby_all() -> void
  {
    set_standby_all(LOW);
  }
auto Control::error(const String& msg) -> void
  {
    Log::error()("#ERROR:", msg);
    halt();
  }
//============================================================================
// text processing functions
//============================================================================
auto Control::data_to_bool(const String& d) -> pair<bool, bool>
  {
    pair<bool, bool> result(false, false);
    const auto& requested_state = d;
    if ( requested_state == "0" 
      || requested_state == "false")
    {
      result.first  = true;
      result.second = false;
    } 
    else 
    if( requested_state == "1"
     || requested_state == "true")
    {
      result.first  = true;
      result.second = true;
    }
    return result;
  }
auto Control::data_to_int(const String& d) -> pair<bool, int>
  {
    pair<bool, int> result(true, 0);
    // make sure that every character is either a digit, a sign, 
    // or null terminator
    for(auto c : d)
    {
      if((isdigit(c) || c == '-' || c == '+') == false)
      {
        result.first = false;
        break;
      }
    }
    if(result.first == true)
    {
      result.second = atoi(d.c_str()); 
    }
    return result;
  }
auto Control::get_char() -> char
  {
    int ch;
    while((ch = Serial.read()) == -1) { yield(); }
    return ch;
  }
auto Control::get_line() -> String
  {
    while(Serial.peek() == -1);
    return Serial.readStringUntil('\n');
  }
auto Control::peek_char() -> int
  {
    int ch;
    while((ch = Serial.peek()) == -1) { yield(); }
    return static_cast<unsigned char>(ch);
  }
//============================================================================
// errors and logging
//============================================================================
auto Control::error_expected_bool(const String& data) -> void
  {
    Log::error()("Expected a boolean value, but got: ", data);
  }
auto Control::error_expected_int(const String& data) -> void
  {
    Log::error()("Expected an integer, but got: ", data);
  }
//============================================================================
//============================================================================
auto Control::run_command_processor() -> void
  {
    // get and parse command line
    auto cmdline = get_line();
    Log::debug()("Received command line: \"", cmdline, "\"");
    auto rcode = rcode_t::parse(cmdline); 
    Log::debug()
      ( "Parsed as:["
      , rcode.name()
      , "]["
      , (int)rcode.command()
      , "]["
      , rcode.data()
      , "]\n"
      );
    if(rcode.error() != rcode_t::Error::ok)
    {
      switch(rcode.error())
      {
      case rcode_t::Error::expected_end_of_line:
        Log::error()("Error: expected end of line");
        break;
      case rcode_t::Error::expected_identifier:
        Log::error()("Error: expected identifier");
        break;
      case rcode_t::Error::invalid_data:
        Log::error()("Error: invalid data");
        break;
      default:
        Log::error()("rcode invalid (bad subcommand)");
      }
      return;
    }
    if(rcode.command() == rcode_t::Command::invalid)
    {
    }
    // search for requested function by name
    const map_entry* fn_entry = nullptr;
    for_each_function(
      [&] (auto& f)
        {
          if(fn_entry == nullptr)
          {
            if(rcode.name() == f.name)
            {
              fn_entry = &f; 
            }
          }
        }
    );
    auto command_not_found = fn_entry == nullptr;
    if(command_not_found)
    {
      Log::debug()("Command \"", rcode.name(), "\" not recognized.");
      return;
    }
    else
    {
      auto callback = fn_entry->callback;
      if(callback != nullptr)
      {
        (this->*(callback))(rcode);
      }
      else
      {
        Log::warning()("Command does not map to function; no action taken.");
      }
    }
  }
auto Control::set_standby_all(pin_value_t pv) -> void
  {
    platform_.set_standby(pv);
    carriage_.set_standby(pv);
  }
//============================================================================
// "rc" functions
//============================================================================
auto Control::rc_carriage_move_steps(const rcode_t& rc) -> void
  {
    auto result = data_to_int(rc.data());
    if(result.first == true)
    {
      const auto& steps = result.second;
      Log::info()("Moving carriage ", steps, " steps");
      auto_standby(carriage_, [&]
        {
          carriage_.set_speed(config_.carriage_speed_);
          carriage_.step(steps);
        }
      );
    }
    else
    {
      error_expected_int(rc.data());
      halt();
    }
  }
auto Control::rc_carriage_set_home(const rcode_t& rc) -> void
  {
    auto_set_home();
  }
auto Control::rc_carriage_set_span(const rcode_t& rc) -> void
  {
    auto_set_home();
    auto_set_max();
    Log::info()("carriage_max_ == ", config_.carriage_max_);
  }
auto Control::rc_log_info(const rcode_t& rc) -> void
  {
    auto set_logger_state = [&](const auto& lname, bool requested_state)
      {
        auto set_for_logger = [&](auto& logger)
        {
          constexpr bool enabled = true;
          const char* state_string = requested_state == enabled? "enabled" : "disabled";
          logger.set_enabled(requested_state);
          Log::info()("Logging ", state_string, " for \"", lname, "\" messages.");
        };
        if(lname == "error")   { set_for_logger(Log::error());   }
        if(lname == "warning") { set_for_logger(Log::warning()); }
        if(lname == "info")    { set_for_logger(Log::info());    }
        if(lname == "debug")   { set_for_logger(Log::debug());   }
      };
    auto last_dot = rc.name().lastIndexOf('.');
    if(last_dot <0 || static_cast<unsigned>(last_dot) == rc.name().length())
    {
      Log::error()("Parse error; suffix not found"); 
      return;
    }
    auto logger_name = rc.name().substring(last_dot + 1); 
    auto result = data_to_bool(rc.data());
    if(result.first == true)
    {
      const auto& requested_state = result.second;
      set_logger_state(logger_name, requested_state);
    }
    else
    {
      error_expected_bool(rc.data());
    }
  }
auto Control::rc_platform_move_steps(const rcode_t& rc) -> void
  {
    auto result = data_to_int(rc.data());
    if(result.first == true)
    {
      const auto& steps = result.second;
      Log::info()("Moving platform ", steps, " steps");
      platform_.resume();
      platform_.set_speed(config_.platform_speed_);
      platform_.step(steps);
      platform_.standby();

//      auto_standby(platform_, [&]
//        {
//          platform_.set_speed(config_.platform_speed_);
//          platform_.step(steps);
//        }
//      );
    }
    else
    {
      error_expected_int(rc.data());
      halt();
    }
  }
auto Control::rc_platform_speed(const rcode_t& rc) -> void
  {
    auto do_get_speed = [&]
      {
        Serial.println(config_.platform_speed_);
      };
    auto do_set_speed = [&]
      {
        auto result = data_to_int(rc.data());
        if(result.first == true)
        {
          config_.platform_speed_ = result.second;
        }
        else
        {
          Log::error()("Could not set speed; argument conversion error.");
        }
      };
    switch(rc.command())
    {
    case rcode_t::Command::get:
      do_get_speed();
      break;
    case rcode_t::Command::set:
      do_set_speed(); 
      do_get_speed(); 
      break;
    default:
      Log::error()("Invalid subcommand");
    }
  }
auto Control::rc_rangefinder_ping(const rcode_t& rc) -> void
  {
    VL53L0X_RangingMeasurementData_t measure;
    Log::info()("Single range measurement");
    tof_sensor_.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) {  // phase failures have incorrect data
      Serial.println(measure.RangeMilliMeter);
    } else {
      Log::info()("Out of range ");
    }
  }
auto Control::rc_reboot(const rcode_t& rc) -> void
  {
    reboot();
  }
auto Control::rc_system_poll(const rcode_t& rc) -> void
  {
    for_each_function(
      [&](auto& f)
        {
          Serial.println(f.name);
        }
    );
  }
//============================================================================
// "test" functions
//============================================================================
auto Control::test_limit_switches() -> void
  {
    Serial.println("*** LIMIT SWITCH TEST ***");
    while(true)
    {
      yield();
      if(limit_reached(limit_switch_min_))
      {
        Serial.println("Min limit");
      }
      if(limit_reached(limit_switch_max_))
      {
        Serial.println("Max limit");
      }
    }
  }
