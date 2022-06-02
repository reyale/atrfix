#pragma once

#include <charconv>

#include "atrfix/consts.h"


namespace atrfix {

  template < typename Func >
  concept token_parse_func = requires(Func fun) {
    { fun(int(), "test", size_t()) };
  };

  template < token_parse_func OnField >
  void parse_message(const char* msg, size_t len, OnField field_func) {
    const char* token_start = msg;
    int tag = -1;

    for(size_t i = 0; i < len; ++i) {
      char working = msg[i];
      if(tag == -1) {
        if(working == '=') {
          size_t tag_len = ((msg +i) - token_start);
          auto result = std::from_chars(token_start, msg + i, tag); //from_chars seems a lot faster that stoi, ect
          if(result.ec != std::errc())
            return;

          token_start = msg + i + 1; 
        }

        continue;
      }

      if(msg[i] == '\001') {
        field_func(tag, token_start, ((msg + i)-token_start));

        token_start = msg + i + 1;
        tag = -1;
      }
    }
  }

  template < typename Container >
  void hash_parse(const char* msg, size_t len, Container & container) {
    parse_message(msg, len, [&container](int tag, const char* start, size_t len) {
       container[tag] = Container::value_type(start, len); //if there are duplicate tags this gets the last one, but that's probably fine
    }); 
  }

  inline seqno parse_seqno_value(const char* field, size_t len) {
    if(field == nullptr || len <= 0)
      return -1;

    seqno s;
    auto res = std::from_chars(field, field+len, s);
    if(res.ec != std::errc())
      return -1;

    return s; 
  }

  template < typename Transform >
  auto parse_singular_field(const char* msg, size_t len, Transform transform) {
    const char* token_start = msg;
    int tag = -1;

    for(size_t i = 0; i < len; ++i) {
      char working = msg[i];
      if(tag == -1) { 
        if(working == '=') {
          size_t tag_len = ((msg +i) - token_start); 
          auto result = std::from_chars(token_start, msg + i, tag); //from_chars seems a lot faster that stoi, ect
          if(result.ec != std::errc())
            return std::make_tuple(-1, transform(nullptr, 0)); 
          
          token_start = msg + i + 1;
        }
        
        continue;
      }

      if(msg[i] == '\001')
        return std::make_tuple(tag, transform(token_start, ((msg + i)-token_start)));
    }

    return std::make_tuple(-1, transform(nullptr, 0)); 
  }

  struct __attribute__((__packed__)) msgtype1 {
    char delim;
    char three;
    char five;
    char equals;
    char type;
    char end_delim;
  };

  struct __attribute__((__packed__)) msgtype2 {
    char delim;
    char three;
    char five;
    char equals;
    char type_one;
    char type_two;
    char end_delim;
  };

  inline char parse_msg_type(const char* buffer) {
    auto msg1 = reinterpret_cast<const msgtype1*>(buffer);
    if(msg1->end_delim == consts::FIELD_DELIM) {
      return msg1->type; 
    }

    auto msg2 = reinterpret_cast<const msgtype2*>(buffer);
    if(msg2->end_delim == consts::FIELD_DELIM) {
      //TODO - unhandled for now
      return consts::msgtype::INVALID; 
    }

    return consts::msgtype::INVALID;
  }

}
