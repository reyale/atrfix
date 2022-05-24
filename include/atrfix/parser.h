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
