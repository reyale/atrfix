#pragma once

#include <chrono>

#include "atrfix/clock.h"
#include "atrfix/parser.h"
#include "atrfix/message.h"
#include "atrfix/seqnum.h"


namespace atrfix {
 
  template < clock_interface clock, seqnum_store_interface seqno_store, typename implementation > 
  class session {
  public:
    session(const std::string & sendercomp, const std::string& targetcomp, const std::string & beginstr=consts::beginstrs::FIX44) 
        : _connected(false), _logged_in(false), 
        _heartbeat_msg(beginstr, sendercomp, targetcomp), 
        _logon_msg(beginstr, sendercomp, targetcomp),
        _sequence_reset(beginstr, sendercomp, targetcomp),
        _hb_interval(_clock.from_seconds(45)) { }

    //for child class to implement
    void disconnect() { }

    template < typename Msg >
    void send_message(const Msg & msg) {
    }

    void maintain_connection() {
      if(!_connected) {
        static_cast<implementation*>(this)->connect(); 
        return;
      }

      if(!_logged_in) {
        send_logon(); 
        return;
      }

      auto current_time = _clock.current_time();
      if((current_time - _last_seen_msg) > _hb_interval) {
        _connected = false;
        _logged_in = false;
        static_cast<implementation*>(this)->disconnect(); 
      }

      if((current_time - _last_sent_msg) > _hb_interval) {
        static_cast<implementation*>(this)->send_message(_heartbeat_msg);
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
        auto seqno_location = view.find("\00134=");
        if(seqno_location == std::string_view::npos) {
          static_cast<implementation*>(this)->disconnect();
          return;
        }

        auto [tag, seqno] = parse_singular_field(working_loc + seqno_location + 1, ((loc+8) - seqno_location -1), parse_seqno_value);
        if(tag == -1 || seqno == -1) {
          static_cast<implementation*>(this)->disconnect();
          return;
        }

        if(seqno > _recv_seqno.current()) {
          //send replay request, for now disconnect
          static_cast<implementation*>(this)->disconnect();
          return;
        }

        if(seqno < _recv_seqno.current())
          _recv_seqno.set(seqno+1); //if we're ahead we just accept
        else
          _recv_seqno.increment();

        auto msg_type_location = view.find("\00135=");
        if(msg_type_location == std::string_view::npos) {
          static_cast<implementation*>(this)->disconnect();
          return; 
        }

        char msgtype = parse_msg_type(working_loc + msg_type_location);
        if(msgtype == consts::msgtype::INVALID) {
          static_cast<implementation*>(this)->disconnect();
          return; 
        }

        if(msgtype == consts::msgtype::Logout) {
          static_cast<implementation*>(this)->disconnect();
          return;
        }

        if(msgtype == consts::msgtype::ResendRequest) { //always reset seqno instead of replaying to counterparty, orders would be stale
          auto endseqno_loc = view.find("\00116=");
          if(endseqno_loc == std::string_view::npos) {
            static_cast<implementation*>(this)->disconnect();
            return;
          }

          auto [endseqno_tag, endseqno] = parse_singular_field(working_loc + endseqno_loc + 1, ((loc+8) - endseqno_loc -1), parse_seqno_value);
          if(endseqno_tag == -1 || endseqno == -1) {
            static_cast<implementation*>(this)->disconnect();
            return;
          }

          _sequence_reset.reset();
          _sequence_reset.set_field(atrfix::fields::GapFillFlag, 'N');
          _sequence_reset.set_field(atrfix::fields::NewSeqNo, endseqno + 1);
          static_cast<implementation*>(this)->send_message(_sequence_reset);
          _send_seqno.set(endseqno + 1); //the next thing we will send is beyond the end of the replay request
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
    void send_logon() {
      if(!_connected)
        return;

      if(!_logged_in)
        static_cast<implementation*>(this)->send_message(_logon_msg); 
    }

    clock _clock;
    clock::timestamp _last_seen_msg;
    clock::timestamp _last_sent_msg;
    clock::timedelta _hb_interval;
    bool _connected;
    bool _logged_in;
    atrfix::heartbeat _heartbeat_msg;
    atrfix::logon _logon_msg;
    atrfix::sequence_reset _sequence_reset;
    seqno_store _send_seqno;
    seqno_store _recv_seqno; 
  };

}
