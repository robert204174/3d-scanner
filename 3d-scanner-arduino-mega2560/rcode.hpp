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
#ifndef rcode_hpp_20200615_113748_PDT
#define rcode_hpp_20200615_113748_PDT

#include <stddef.h>
#include <Arduino.h>

template<typename T, typename U>
class RCode final
{
  static constexpr char set_command_prefix_ = '+';
  static constexpr char get_command_prefix_ = '-';
  static constexpr char delimiter_          = ':';
public:
  using target_t    = T;
  using string_t    = U;
  using getter_t    = auto(target_t::*)(void) -> const string_t&;
  using setter_t    = auto(target_t::*)(const string_t&) -> void;

  enum class Command { Invalid, Get, Set };

  RCode(const RCode&) = delete;
  RCode(RCode&&) = delete;

  RCode(const string_t& n, Command c, const string_t& d) noexcept
  : name_(n)
  , command_(c)
  , data_(d)
    {
    }
  RCode(const string_t& n, Command c) noexcept
  : RCode(n, c, "")
    {
    }
  RCode(const string_t& n) noexcept
  : RCode(n, Command::Invalid)
    {
    }
  RCode() noexcept
  : RCode("")
    {
    }
  ~RCode() noexcept {}

  auto name() const           -> const string_t&  { return name_; }
  auto command() const        -> const Command&   { return command_; }
  auto data() const           -> const string_t&  { return data_; }
  static auto parse(const string_t& s) -> RCode 
    {
      Command command = Command::Invalid;
      string_t name;
      string_t data;
      enum Mode { mode_error, mode_command, mode_name, mode_data } mode = mode_command;
      using sz_t = decltype(s.length());
      using char_t = decltype(s[0]);
      for(sz_t i = 0; i < s.length() && mode != mode_error; ++i)
      {
        char_t ch = s[i];
        switch(mode)
        {
        case mode_command:
          switch(ch)
          {
          case set_command_prefix_:
            command = Command::Set; 
            mode = mode_name;
            break;
          case get_command_prefix_:
            command = Command::Get;
            mode = mode_name;
            break;
          default:
            command = Command::Invalid;
            mode = mode_error;
          }
          break;
        case mode_name:
          if(isalnum(ch) || ch == '_' || ch == '.')
          {
            name += ch;
          }
          else if(ch == delimiter_)
          {
            mode = mode_data;
          }
          break;
        case mode_data:
          data += ch;
          break;
        default: 
          break;
        }
      }
      return RCode(name, command, data);
    }
private:
  string_t    name_;
  Command     command_;
  string_t    data_;
};

#endif//rcode_hpp_20200615_113748_PDT
