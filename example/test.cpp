#include <iostream>
#include <cassert>

#include "atrfix/message.h"
#include "atrfix/fields.h"
#include "atrfix/checksum.h"


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

  {
      //8=FIX.4.2|9=49|35=5|34=1|49=ARCA|52=20150916-04:14:05.306|56=TW|10=157|
      const std::string msg = "8=FIX.4.2\0019=49\00135=5\00134=1\00149=ARCA\00152=20150916-04:14:05.306\00156=TW\001";

      unsigned int total = 0;
      total = atrfix::add_to_checksum(total, msg.data(), msg.size());

      int target_checksum = 157;
      assert(target_checksum == total % atrfix::consts::CHECKSUM_MOD);

      std::string target_checksum_str("157");
      assert(target_checksum_str == atrfix::render_checksum(total));
  }



  return 0;
}
