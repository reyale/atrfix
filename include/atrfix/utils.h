#pragma once

#include <strings.h>


namespace atrfix {

  template < size_t T >
  struct fixed_str_buffer {
    constexpr static size_t LEN = T;

    fixed_str_buffer() { ::bzero(data, LEN); }
    char data[LEN]; 
  };

}
