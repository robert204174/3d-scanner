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
#ifndef stepper_control_hpp_20200611_174745_PDT
#define stepper_control_hpp_20200611_174745_PDT

#include "logger.hpp"
#include <Stepper.h>
#include <Arduino.h>
template
  < class T 
  , int STEPS
  , int CA1
  , int CA2
  , int CB1
  , int CB2
  , int STBY
  >
class StepperControl
{
public:
  using stepper_t   = T;
  using pin_value_t = decltype(HIGH);
  StepperControl()
    : stepper_(STEPS, CA1, CA2, CB1, CB2)
  {
  }

  auto begin() -> void
    {
      pinMode(STBY, OUTPUT);
      standby();
    }
  static constexpr auto steps_per_revolution() -> auto { return STEPS; }

  auto set_speed(int s) -> void 
    { 
      speed_ = s;
      stepper_.setSpeed(speed_); 
    }

  auto speed() const -> const int { return speed_; }
  auto step(int n) -> void 
    { 
      stepper_.step(n); 
    }
  auto set_standby(pin_value_t pv) -> void 
  { 
    digitalWrite(STBY, pv); 
  }
  auto standby() -> void 
  { 
    set_standby(LOW);
  }
  auto resume() -> void
  {
    set_standby(HIGH); 
  }
  auto stepper_inactive() -> bool { return digitalRead(STBY) == LOW; }
private:
  stepper_t stepper_;
  int       speed_ = 0;
};

#endif//stepper_control_hpp_20200611_174745_PDT
