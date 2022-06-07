#include <iostream>
#include <cassert>

#include "atrfix/message.h"
#include "atrfix/fields.h"
#include "atrfix/checksum.h"
#include "atrfix/message.h"
#include "atrfix/parser.h"


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
    std::string result("8=10\001");
    assert(std::string_view(buffer_dest, written) == result);
  }

  {
    char buffer_dest[25];
    size_t written = atrfix::write_field(buffer_dest, atrfix::fields::MsgType, 1123.45);
    std::string result("35=1123.45\001");
    assert(std::string_view(buffer_dest, written) == result);
  }

  {
    char buffer_dest[30];
    size_t written = atrfix::write_field(buffer_dest, atrfix::fields::Text, "this is a message"); 
    std::string result("58=this is a message\001");
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
    time_t time = 1652925127;
    unsigned int seqno = 124;

    const std::string msg = "8=FIX.4.4\0019=0065\00135=D\00149=TESTBUY1\00156=TESTSELL1\00134=000124\00152=20220519-01:52:07.306\00110=064\001"; 
    atrfix::message fixmsg("8=FIX.4.4", 'D', "TESTBUY1", "TESTSELL1");

    auto sent_msg = sendv(fixmsg.render(seqno, time));

    assert(sent_msg == msg);

    //set a field

    const std::string msg2 = "8=FIX.4.4\0019=0074\00135=D\00149=TESTBUY1\00156=TESTSELL1\00134=000124\00152=20220519-01:52:07.306\00144=10.50\00110=218\001";
    atrfix::message fixmsg2("8=FIX.4.4", 'D', "TESTBUY1", "TESTSELL1");
    fixmsg2.set_field(atrfix::fields::Price, 10.50);

    sent_msg = sendv(fixmsg2.render(seqno, time));
    assert(sent_msg == msg2);

    //reuse the message
    fixmsg2.reset();
    fixmsg2.set_field(atrfix::fields::Price, 10.50);
    sent_msg = sendv(fixmsg2.render(seqno, time));
    assert(sent_msg == msg2);

  } 

  {
    std::vector<std::pair<int, std::string>> result;
    auto append_result = [&result](int tag, const char* val, size_t val_len) {
      result.push_back(std::make_pair(tag, std::string(val, val_len)));
    };

    const std::string single_field("8=FIX.4.4\001");
    atrfix::parse_message(single_field.c_str(), single_field.size(), append_result); 

    assert(result.size() == 1);
    assert(result[0].first == 8);
    assert(result[0].second == "FIX.4.4");

    result.clear();
    const std::string multi_field("8=FIX.4.4\0019=0071\001");
    atrfix::parse_message(multi_field.c_str(), multi_field.size(), append_result);

    assert(result.size() == 2);
    assert(result[0].first == 8);
    assert(result[0].second == "FIX.4.4");
    assert(result[1].first == 9);
    assert(result[1].second == "0071");

  }

  return 0;
}
