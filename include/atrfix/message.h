#pragma once

#include <sys/uio.h>
#include <string>
#include <string.h>
#include <stdexcept>
#include <time.h>

#include "fmt/format.h"
#include "fmt/chrono.h"

#include "atrfix/utils.h"
#include "atrfix/consts.h"
#include "atrfix/checksum.h"


namespace atrfix {

  size_t write_val(char* dest, int val) {
    auto end = fmt::format_to(dest, "{}", val);
    return end - dest;
  }

  size_t write_val(char* dest, double val) {
    auto end = fmt::format_to(dest, "{:.2f}", val);
    return end - dest;
  }

  size_t write_val(char* dest, time_t val) {
    //20150916-04:14:05.306
    auto end = fmt::format_to(dest, "{:%Y-%m-%d-%H:%M:%S}", fmt::gmtime(val));
    return end - dest;
  }

  template < size_t N, typename T >
  size_t write_field(char* dest, const char(&field_name)[N], T && t) {
    ::strncpy(dest, field_name, N - 1);
    char* loc = dest + (N - 1);
    *loc = '=';
    return N + write_val(dest + N, t);
  }

  using footer_t = fixed_str_buffer<consts::FOOTER_LEN>;

  struct ioresult {
    struct iovec iov[3];
  };

  template < size_t BODY_SIZE = 1024 >
  class message {
  public:
    message(const std::string & beginstr, char msgtype, const std::string & sender_comp_id, const std::string & target_comp_id) :  checksum_start_total(0), body_len(0), msglen_write_location(0) {

      /*
      8 	BeginString 	Y 	
      9 	BodyLength 	Y 	
      35 	MsgType 	Y 	
      49 	SenderCompID 	Y 	
      56 	TargetCompID 	Y 
      34 	MsgSeqNum 
      52 	SendingTime */

      _header = fmt::format("{}\0019=000\00135={}\00149={}\00156={}\00134=000000\00152={}\001", beginstr.c_str(), msgtype, sender_comp_id.c_str(), target_comp_id.c_str(), "20150916-04:14:05.306");

      /*
      _header += beginstr;
      _header += "9=0000" + consts::FIELD_DELIM;
      _header += "35=" + msgtype + consts::FIELD_DELIM;
      _header += "49=" + sender_comp_id + consts::FIELD_DELIM;
      _header += "56=" + target_comp_id + consts::FIELD_DELIM;
      _header += "34=000000" + consts::FIELD_DELIM;
      _header += "52=20150916-04:14:05.306" + consts::FIELD_DELIM; 
      */

      //location to write the message length to
      msglen_write_location = find_write_location(_header.data(), "9=");
      if(msglen_write_location == nullptr)
        throw std::runtime_error("failed to find location to write msg length");

      seqno_write_location = find_write_location(_header.data(), "34=");
      if(seqno_write_location == nullptr)
        throw std::runtime_error("failed to find location to write seqno");

      sendtime_write_location = find_write_location(_header.data(), "52=");
      if(sendtime_write_location == nullptr)
        throw std::runtime_error("failed to find location to write sendtime");

      ::strncpy(footer.data, consts::EXAMPLE_FOOTER, footer_t::LEN);

      body = new char[BODY_SIZE]; 

      auto& iov = _result.iov;
      iov[0].iov_base = _header.data();
      iov[0].iov_len = _header.size();
      iov[1].iov_base = body;
      iov[1].iov_len = body_len;
      iov[2].iov_base = footer.data;
      iov[2].iov_len = footer_t::LEN; 
    }

    ~message() { delete body; }

    const ioresult& render(unsigned int seqno, time_t time_utc) {
      if(body_len > BODY_SIZE)
        throw std::runtime_error("fail to render message, length is too large to express"); 

      if(seqno > 999999)
        throw std::runtime_error("failed to render message, seqno too large");

      fmt::format_to(msglen_write_location, "{:04d}", body_len);
      fmt::format_to(seqno_write_location, "{:06d}", seqno);
      fmt::format_to(sendtime_write_location, "{:%Y%m%d-%H:%M:%S}", fmt::gmtime(time_utc));

      //write checksum
      unsigned int total = atrfix::add_to_checksum(0, _header.data(), _header.size());
      total = add_to_checksum(total, body, body_len);
      const char* checksum = atrfix::render_checksum(total);
      ::strncpy(footer.data + 3, checksum, 3);
     
      _result.iov[2].iov_len = body_len;
      return _result;
    }

    void reset() {
      body_len = 0;
    }

    char* find_write_location(const char* buffer, const std::string& tag) {
      std::string_view view(buffer);
      auto iter = view.find(tag);
      if(iter == std::string_view::npos) 
        return nullptr;

      return const_cast<char*>(buffer + iter + tag.size()); 
    }

    template < size_t N, typename T >
    void set_field(const char(&field_name)[N], T && t) {
      body_len += write_field(body + body_len, field_name, t);
    }

  protected:
    unsigned int checksum_start_total;
    std::string _header;

    char* body;
    size_t body_len;

    footer_t footer;

    char* msglen_write_location = nullptr;
    char* seqno_write_location = nullptr;
    char* sendtime_write_location = nullptr;
    struct ioresult _result;
  };

}
