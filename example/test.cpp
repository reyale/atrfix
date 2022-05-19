#include <iostream>
#include <cassert>

#include "atrfix/message.h"
#include "atrfix/fields.h"
#include "atrfix/checksum.h"
#include "atrfix/message.h"


std::string 
sendv(const atrfix::ioresult & result) {
  return fmt::format("{}{}{}", std::string_view((char*)result.iov[0].iov_base, result.iov[0].iov_len), 
                               std::string_view((char*)result.iov[1].iov_base, result.iov[1].iov_len), 
                               std::string_view((char*)result.iov[2].iov_base, result.iov[2].iov_len));
}

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

  {

    unsigned int seqno = 124;
    const std::string msg = "8=FIX.4.4\0019=0000\00135=D\00149=TESTBUY1\00156=TESTSELL1\00134=000124\00152=20220519-01:52:07.306\001"; //44=44.40\00153=100\00158=TEST message\001";
    atrfix::message fixmsg("8=FIX.4.4", 'D', "TESTBUY1", "TESTSELL1");

    time_t time = 1652925127;
    auto sent_msg = sendv(fixmsg.render(seqno, time));
    std::cout << msg << std::endl;
    std::cout << sent_msg << std::endl;

    assert(sent_msg == msg);
  }

  return 0;
}
