#include "control.hpp"
#include "Adafruit_VL53L0X.h"
#include "logger.hpp"
#include "rcode.hpp"
#include <avr/wdt.h>
#include <Arduino.h>

namespace 
  {
    Adafruit_VL53L0X tof_sensor {};
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
    if(tof_sensor.begin() == failure)
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
        Serial.println("Invalid mode requested");
      }
      halt();
    }
  }
auto Control::reboot() -> void
  {
    Log::info()("Rebooting...\n");
    delay(reboot_delay_);
    // start watchdog with the provided prescaller
    wdt_enable(WDTO_15MS);
    // wait for the prescaller time to expire
    // without sending the reset signal by using
    // the wdt_reset() method
    while(1) {}
  }
auto Control::halt() -> void
  {
    Serial.println("#break");
    standby_all();
    Serial.println("#terminate");
    Log::info()("Type @reboot to reboot device\n");
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
    Log::info()("Searching for home...\n");
    auto count = seek_limit(limit_switch_max_, SeekOrientation::Forward);
    Log::info()("Homing complete.\n");
    config_.carriage_max_ = count;
    return count;
  }
auto Control::auto_set_home() -> seek_count_t
  {
    Log::info()("Searching for home...\n");
    auto count = seek_limit(limit_switch_min_, SeekOrientation::Reverse);
    Log::info()("Homing complete.\n");
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
auto Control::get_line() -> String
  {
    while(Serial.peek() == -1);
    return Serial.readStringUntil('\n');
  }
auto Control::get_char() -> char
  {
    int ch;
    while((ch = Serial.read()) == -1) { yield(); }
    return ch;
  }
auto Control::peek_char() -> int
  {
    int ch;
    while((ch = Serial.peek()) == -1) { yield(); }
    return static_cast<unsigned char>(ch);
  }
auto Control::error(const String& msg) -> void
  {
    Log::error()("#ERROR:", msg);
    halt();
  }
auto Control::run_command_processor() -> void
  {
    // structure for mapping rcodes to member functions
    struct map_entry
    {
      using setter_t = auto(Control::*)(const rcode_t&) -> void;
      using getter_t = auto(Control::*)(const rcode_t&) -> const String&;
      String    name;
      setter_t  set_fn;
      getter_t  get_fn;
    };
    // array of rcode mapping
    static map_entry rcode_map[] = 
      {
        map_entry { "carriage.move.steps"     , &Control::rc_carriage_move_steps, nullptr },
        map_entry { "carriage.auto_set_home"  , &Control::rc_carriage_set_home  , nullptr },
        map_entry { "carriage.auto_set_span"  , &Control::rc_carriage_set_span  , nullptr },
//        map_entry { "log.info"                , &Control::rc_log_info           , nullptr }
        map_entry { "platform.move.steps"     , &Control::rc_platform_move_steps, nullptr },
        map_entry { "reboot"                  , &Control::rc_reboot             , nullptr }
      };
    // get and parse command line
    auto cmdline = get_line();
    {
      Log::debug()("Received command line: \"", cmdline, "\"");
    }
    auto rcode = rcode_t::parse(cmdline); 
    {
      Log::debug()
        ( "Parsed as:["
        , rcode.name()
        , "]["
        , (int)rcode.command()
        , "]["
        , rcode.data()
        , "]"
        );
    }
    // search for requested function by name
    bool command_not_found = true;
    for(auto& m : rcode_map)
    {
      bool invalid = false;
      if(rcode.name() == m.name)
      {
        command_not_found = false;
        switch(rcode.command())
        {
        case rcode_t::Command::Invalid:
          invalid = true;
          break;
        case rcode_t::Command::Get:
          Log::debug()("Executing 'get' command for register:", rcode.name(), '\n');
          if(m.get_fn)
          {
            Serial.println((this->*(m.get_fn))(rcode));
          }
          break;
        case rcode_t::Command::Set:
          Log::debug()("Executing 'set' command for register:", rcode.name(), "\" with data = \"", rcode.data(), "\"\n");
          if(m.set_fn)
          {
            (this->*m.set_fn)(rcode);
          }
          break;
        }
      }
      if(invalid) 
      { 
        break; // from for loop
      }
    }
    if(command_not_found)
    {
      Log::debug()("Command \"", rcode.name(), "\" not recognized.\n");
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
    auto steps = atoi(rc.data().c_str());
    Log::info()("Moving carriage ", steps, " steps\n");
    carriage_.resume();
    carriage_.set_speed(config_.carriage_speed_);
    carriage_.step(steps);
    carriage_.standby();
  }
auto Control::rc_platform_move_steps(const rcode_t& rc) -> void
  {
    auto steps = atoi(rc.data().c_str());
    Log::info()("Moving platform  ", steps, " steps\n");
    platform_.resume();
    platform_.set_speed(config_.platform_speed_);
    platform_.step(steps);
    platform_.standby();
  }
auto Control::rc_carriage_set_home(const rcode_t& rc) -> void
  {
    auto_set_home();
  }
auto Control::rc_carriage_set_span(const rcode_t& rc) -> void
  {
    auto_set_home();
    auto_set_max();
    Log::info()("carriage_max_ == ", config_.carriage_max_, "\n");
  }
auto Control::rc_reboot(const rcode_t& rc) -> void
  {
    reboot();
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
