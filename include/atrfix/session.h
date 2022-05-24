#pragma once

#include <chrono>

#include "atrfix/clock.h"
#include "atrfix/parser.h"


namespace atrfix {
 
  template < clock_interface clock, typename implementation > 
  class session {
  public:
    session() : _connected(false), _logged_in(false) { _hb_interval = _clock.from_seconds(30); }

    //for child class to implement
    void disconnect() { }
    void send_heartbeat() { }

    void maintain_connection() {
      if(!_connected) {
        static_cast<implementation*>(this)->connect(); 
        return;
      }

      if(!_logged_in) {
        static_cast<implementation*>(this)->send_logon(); 
        return;
      }

      auto current_time = _clock.current_time();
      if((current_time - _last_seen_msg) > _hb_interval) {
        _connected = false;
        _logged_in = false;
        static_cast<implementation*>(this)->disconnect(); 
      }

      if((current_time - _last_sent_msg) > _hb_interval) {
        static_cast<implementation*>(this)->send_heartbeat();
        _last_sent_msg = current_time;
      }
    }

    void handle_read(const char* read, size_t size) {
      const char* working_loc = read;
      while(true) { //TCP stream, can recieve several messages
        auto view = std::string_view(working_loc, size - (working_loc - read));
        auto loc = view.find("\00110=");
        if(loc == std::string_view::npos)
          break;

        if((size-loc) < 8) //incomplete message, checksum not finished writing
          break;

        if(!valid_checksum(working_loc, loc + 8)) {
          working_loc += loc + 8;
          continue; 
        }

        view = std::string_view(working_loc, loc+8);
        //validate seqno
        //TODO        

        auto msg_type_location = view.find("\00135=");
        if(msg_type_location == std::string_view::npos) {
          static_cast<implementation*>(this)->disconnect();
          return; 
        }

        char msgtype = parse_msg_type(working_loc + msg_type_location);
        if(msgtype == consts::msgtype::INVALID) {
          working_loc += loc + 8;
          continue;
        }

        if(msgtype == consts::msgtype::Logout) {
          static_cast<implementation*>(this)->disconnect();
          return;
        }

        //handle session login
        if(msgtype = consts::msgtype::Logon)
          _logged_in = true;

        _last_seen_msg = _clock.current_time();
        static_cast<implementation*>(this)->on_message(working_loc, loc+8); 
        working_loc += loc + 8;
      }
    }

    bool valid_checksum(const char* loc, size_t size) {
      return true; //do not validate checksum
    }

    bool session_ready() {
      return _connected && _logged_in;
    }

  protected:
    clock _clock;
    clock::timestamp _last_seen_msg;
    clock::timestamp _last_sent_msg;
    clock::timedelta _hb_interval;
    bool _connected;
    bool _logged_in; 
  };

}
