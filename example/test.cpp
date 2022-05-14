#include <iostream>
#include <cassert>

#include "atrfix/message.h"
#include "atrfix/fields.h"


int main() { 

  {
    char buffer_dest[25];
    size_t written = atrfix::write_field(buffer_dest, atrfix::fields::BeginString, 10);
    std::string result("8=10");
    assert(std::string_view(buffer_dest, written) == result);
  }

  {
    char buffer_dest[25];
    size_t written = atrfix::write_field(buffer_dest, atrfix::fields::MsgType, 1123.45);
    std::string result("35=1123.45");
    assert(std::string_view(buffer_dest, written) == result);
  }

  return 0;
}
