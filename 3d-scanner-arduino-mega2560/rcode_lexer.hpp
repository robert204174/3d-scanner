#ifndef rcode_lexer_hpp_20200621_145638_PDT
#define rcode_lexer_hpp_20200621_145638_PDT

#include "logger.hpp"

template<typename T>
class rcode_lexer
{
public:
  using string_t      = T;
  using iter_t        = const char*;
  using char_t        = char;
  struct token_t 
    {
      enum id_t 
        { END_OF_LINE
        , FLOAT
        , IDENTIFIER
        , INTEGER
        , INVALID 
        , OP_ASSIGN
        , OP_MINUS
        , STRING
        , SUFFIX
        };
      id_t  id      = INVALID;
      iter_t      begin   = nullptr;
      iter_t      end     = nullptr;
    };
  rcode_lexer(const string_t& s)
  : source_(s)
  , next_(s.c_str())
  , end_(next_ + s.length())
    {}

  auto scan() -> token_t { return do_scan(); }
  static auto get_symbol(const token_t& t) -> string_t { return do_get_symbol(t); }
private:
  static constexpr char_t end_of_line_char  = 0;
  static constexpr char_t dot_char          = '.';
  static constexpr char_t eq_char           = '=';
  static constexpr char_t minus_char        = '-';
  static constexpr char_t plus_char         = '+';
  static constexpr char_t start_quote_char  = '"';
  static constexpr char_t end_quote_char    = '"';
  static constexpr char_t underscore_char   = '_';

  const string_t&   source_;
  iter_t            next_ = nullptr;
  iter_t            end_  = nullptr;

  static auto do_get_symbol(const token_t& token) -> string_t
    {
      string_t result;
      if(token.begin < token.end)
      { 
        for(iter_t i = token.begin; i < token.end; ++i)
        {
          result += *i; 
        }
      }
      return result;
    }
  auto do_scan() -> token_t 
    {
      typename token_t::id_t    token_id = token_t::INVALID;
      iter_t                    token_begin = next_;
      iter_t                    token_end   = next_;
      auto accept = [&](const auto& t)
        {
          token_id  = t; 
          token_end = next_;
        };
      auto peek = [&]
        {
          return next_ >= end_? end_of_line_char : *(next_);
        };
      auto advance = [&]
        {
          ++next_;
        };
      auto update_start = [&]
        {
          token_begin = next_;
        };
      auto skip_whitespace = [&]
        {
          while(isspace(peek()))
          {
            advance();
          }
          update_start();
        };
      auto scan_identifier = [&]
        {
          while
            (  isalpha(peek()) 
            || isdigit(peek()) 
            || peek() == underscore_char
            || peek() == dot_char
            )
          {
            advance();
          }
          accept(token_t::IDENTIFIER);
        };
      auto scan_string = [&]
        {
          while(peek() != end_quote_char)
          {
            if(peek() == end_of_line_char) 
            {
              accept(token_t::INVALID);
              return;
            }
            advance();
          }
          advance();
        };
      auto scan_number = [&]
        {
          auto consume_digits = [&]
            {
              while(isdigit(peek()))
              {
                advance();
              }
            };
          auto scan_float = [&]
            {
              if(isdigit(peek()))
              {
                consume_digits();
                accept(token_t::FLOAT);
              }
              else
              {
                accept(token_t::INVALID);
              }
            };
          if(peek() == minus_char || peek() == plus_char)
          {
            advance();
            if(isdigit(peek()) == false)
            {
              accept(token_t::OP_MINUS);
              return;
            }
          }
          consume_digits();
          if(peek() == dot_char)
          {
            advance();
            scan_float();
          }
          else
          {
            accept(token_t::INTEGER);
          }
        };
      ////////////////////////
      skip_whitespace();
      if(isalpha(peek()))
      {
        advance();
        scan_identifier();
      }
      else if(peek() == eq_char)
      {
        advance();
        accept(token_t::OP_ASSIGN);       
      }
      else if(isdigit(peek()))
      {
        advance();
        scan_number();
      }
      else if(peek() == minus_char || peek() == plus_char)
      {
        advance();
        if(isdigit(peek()))
        {
          scan_number();
        }
        else
        {
          accept(token_t::INVALID);
        }
      }
      else if(peek() == start_quote_char)
      {
        advance();
        update_start();
        scan_string();
      }
      else if(peek() == end_of_line_char)
      {
        accept(token_t::END_OF_LINE);
      }
      Log::debug()
        ( "Token contents: "
        , static_cast<int>(token_id),      ", "
        , static_cast<char>(*token_begin), ", "
        , static_cast<char>(*token_end),   ", "
        );
      return token_t { token_id, token_begin, token_end };
    }
};

#endif//rcode_lexer_hpp_20200621_145638_PDT
