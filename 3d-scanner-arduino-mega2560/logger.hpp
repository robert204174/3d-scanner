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
#ifndef logger_hpp_20200616_120255_PDT
#define logger_hpp_20200616_120255_PDT

#include <Arduino.h>

class Log
{
  Log() {};
public:
  static auto error() -> Log&
    {
      static Log l;
      return l;
    }
  static auto debug() -> Log&
    {
      static Log l;
      return l;
    }
  static auto info() -> Log&
    {
      static Log l;
      return l;
    }
  template<typename...ArgsT>
  auto operator()(ArgsT&&...args) -> void { return write(args...); }
private:
  bool is_enabled_ = true;

  auto enable() -> void
    {
      is_enabled_ = true;
    }
  auto disable() -> void
    { 
      is_enabled_ = false;
    }
  template<typename FirstT>
  auto write(FirstT&& first) -> void
    {
      if(is_enabled_)
      {
        Serial.print(first);
      }
    }
  template<typename FirstT, typename...LastTs>
  auto write(FirstT&& first, LastTs&&...last) -> void
    {
      write(first);
      write(last...); 
    }
};


#endif//logger_hpp_20200616_120255_PDT
