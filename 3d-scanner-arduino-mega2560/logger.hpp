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
