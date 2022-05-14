#pragma once

#include <string>
#include <string.h>

#include "fmt/format.h"

#include "atrfix/utils.h"
#include "atrfix/consts.h"


namespace atrfix {

  using footer_t = fixed_str_buffer<consts::FOOTER_LEN>; 
  
  size_t write_val(char* dest, int val) {
    auto end = fmt::format_to(dest, "{}", val);
    return end - dest;
  }

  size_t write_val(char* dest, double val) {
    auto end = fmt::format_to(dest, "{:.2f}", val);
    return end - dest;
  }

  template < size_t N, typename T >
  size_t write_field(char* dest, const char(&field_name)[N], T && t) {
    ::strncpy(dest, field_name, N - 1);
    char* loc = dest + (N - 1);
    *loc = '=';
    return N + write_val(dest + N, t);
  }

  class message {
    message(const std::string& begin_str, const std::string & sender_comp_id, const std::string & target_comp_id) {
      

      ::strncpy(footer.data, consts::EXAMPLE_FOOTER, footer_t::LEN);  
    }

  protected:
    char* begin_string;
    char* body;
    footer_t footer; 
  };

}
