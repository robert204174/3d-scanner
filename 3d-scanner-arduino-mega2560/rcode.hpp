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

#include "rcode_lexer.hpp"
#include <stddef.h>
#include <Arduino.h>

template<typename T>
class RCode final
{
  static constexpr char set_command_prefix_ = '+';
  static constexpr char get_command_prefix_ = '-';
  static constexpr char delimiter_          = ':';
public:
  using string_t    = T;

  enum class Command { invalid, get, set };
  enum class Error   
    { ok
    , expected_end_of_line
    , expected_identifier 
    , invalid_data 
    };

  RCode(const RCode&) = default;
  RCode(RCode&&) = default;

  RCode(const string_t& n, Command c, const string_t& d) noexcept
  : command_(c)
  , name_(n)
  , data_(d)
    {
    }
  RCode(const string_t& n, Command c) noexcept
  : RCode(n, c, "")
    {
    }
  RCode(const string_t& n) noexcept
  : RCode(n, Command::invalid)
    {
    }
  RCode() noexcept
  : RCode("")
    {
    }
  ~RCode() noexcept {}

  auto name() const           -> const string_t&  { return name_;     }
  auto command() const        -> const Command&   { return command_;  }
  auto data() const           -> const string_t&  { return data_;     }
  auto error() const          -> const Error&     { return error_;    }

  static auto parse(const string_t& source) -> RCode 
    {
      RCode result;
      using lexer_t = rcode_lexer<string_t>;
      using token_t = decltype(lexer_t(source).scan());
      auto lexer = lexer_t(source); 
      auto expect_end_of_line = [&]
        {
          auto eol_token = lexer.scan();
          if( eol_token.id != token_t::END_OF_LINE)
          {
            result.error_ = Error::expected_end_of_line;
          }
        };
      auto expect_data = [&]
        {
          auto data_token = lexer.scan();
          if( data_token.id == token_t::INTEGER
           || data_token.id == token_t::FLOAT
           || data_token.id == token_t::STRING
          )
          {
            result.data_ = lexer.get_symbol(data_token);
          }
          else
          {
            result.error_   = Error::invalid_data;
          }
        };
      auto allow_assignment_op = [&]
        {
          auto assignment_token = lexer.scan();
          if(assignment_token.id == token_t::OP_ASSIGN)
          {
            result.command_ = Command::set;
            expect_data();
          }
          else
          {
            result.command_ = Command::get;
            expect_end_of_line();
          }
        };
      auto expect_identifier = [&]
        {
          auto id_token = lexer.scan();
          if(id_token.id == token_t::IDENTIFIER)
          {
            result.name_ = lexer.get_symbol(id_token);
            allow_assignment_op();
          }
          else if(id_token.id == token_t::INVALID)
          {
            result.error_ = Error::expected_identifier;
          }
        };
      auto start_parsing = [&]
        {
          expect_identifier();
        };
      start_parsing();
      return result;
    }
private:
  Command     command_ = Command::invalid;
  Error       error_   = Error::ok;
  string_t    name_;
  string_t    data_;
};

#endif//rcode_hpp_20200615_113748_PDT
