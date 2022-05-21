#pragma once

#include <charconv>


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

}
