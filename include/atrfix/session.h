#pragma once

#include <chrono>

#include "atrfix/clock.h"
#include "atrfix/parser.h"
#include "atrfix/message.h"
#include "atrfix/seqnum.h"


namespace atrfix {

  constexpr unsigned int HB_INTERVAL = 45;

  template < clock_interface clock, seqnum_store_interface seqno_store, typename logger, typename implementation > 
  class session {
  public:
    session(const std::string & sendercomp, const std::string& targetcomp, logger& log, const std::string & beginstr=consts::beginstrs::FIX44) 
        : _connected(false), _logged_in(false), 
        _heartbeat_msg(beginstr, sendercomp, targetcomp), 
        _logon_msg(beginstr, sendercomp, targetcomp),
        _sequence_reset(beginstr, sendercomp, targetcomp),
        _session_reject(beginstr, sendercomp, targetcomp),
        _hb_interval(_clock.from_seconds(HB_INTERVAL)), 
        _logger(log) { }

    //for child class to implement
    void disconnect() { }

    template < typename Msg >
    void send_message(const Msg & msg) {
    }

    void maintain_connection() {
      if(!_connected) {
        _logger.log("{}\n", "not connected, attempting to connect");
        static_cast<implementation*>(this)->connect(); 
        return;
      }

      if(!_logged_in) {
        _logger.log("{}\n", "connected, but not logged in, attempting to send logon");
        send_logon(); 
        return;
      }

      auto current_time = _clock.current_time();
      if((current_time - _last_seen_msg) > _hb_interval) {
        _logger.log("{}\n", "no message seen in hbinterval disconnecting");
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
          send_session_reject(-1, "cannot parse seqno");
          return;
        }

        if(seqno > _recv_seqno.current()) {
          //TODO send replay request, for now disconnect
          send_session_reject(seqno, "seqno is greater than expected, we don't ask for replays");
          return;
        }

        if(seqno < _recv_seqno.current()) {
          send_session_reject(seqno, "seqno is behind expected");
          return;
        } else
          _recv_seqno.increment();

        auto msg_type_location = view.find("\00135=");
        if(msg_type_location == std::string_view::npos) {
          send_session_reject(seqno, "seqno is greater than expected, we don't ask for replays");
          return; 
        }

        char msgtype = parse_msg_type(working_loc + msg_type_location);
        if(msgtype == consts::msgtype::INVALID) {
          send_session_reject(seqno, "invalid msg type");
          return; 
        }

        if(msgtype == consts::msgtype::Reject) { // session level reject
          _logger.log("{}\n", "we received a session level reject, disconnecting");
          static_cast<implementation*>(this)->disconnect();
          return;
        }

        if(msgtype == consts::msgtype::Logout) {
          _logger.log("{}\n", "we received a logout message, disconnecting"); 
          static_cast<implementation*>(this)->disconnect();
          return;
        }

        if(msgtype == consts::msgtype::TestRequest) {
          _logger.log("{}\n", "we received a test request message, sending heartbeat"); 
          static_cast<implementation*>(this)->send_message(_heartbeat_msg);
          _last_sent_msg = _clock.current_time();
        }

        if(msgtype == consts::msgtype::ResendRequest) { //always reset seqno instead of replaying to counterparty, orders would be stale
          auto endseqno_loc = view.find("\00116=");
          if(endseqno_loc == std::string_view::npos) {
            send_session_reject(seqno, "received a ResendRequest without end seqno"); 
            return;
          }

          auto [endseqno_tag, endseqno] = parse_singular_field(working_loc + endseqno_loc + 1, ((loc+8) - endseqno_loc -1), parse_seqno_value);
          if(endseqno_tag == -1 || endseqno == -1) {
            send_session_reject(seqno, "received a ResendRequest without a parseable end seqno"); 
            return;
          }

          _logger.log("sending sequence_reset with new end_seqno={}\n", endseqno + 1);

          _sequence_reset.reset();
          _sequence_reset.set_field(atrfix::fields::GapFillFlag, 'N');
          _sequence_reset.set_field(atrfix::fields::NewSeqNo, endseqno + 1);
          static_cast<implementation*>(this)->send_message(_sequence_reset);
          _send_seqno.set(endseqno + 1); //the next thing we will send is beyond the end of the replay request
        }

        //handle session login
        if(msgtype == consts::msgtype::Logon) {
          _logger.log("{}\n", "received logon msg");
          _logged_in = true;
        }

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

    template < size_t N >
    void send_session_reject(seqno invalid_msg_seqno, const char(&msg)[N]) {
      _logger.log("sending session reject and disconnecting: seqno={} msg={}\n", invalid_msg_seqno, msg);
      _session_reject.reset();
      _session_reject.set_field(atrfix::fields::RefSeqNum, invalid_msg_seqno);
      _session_reject.set_field(atrfix::fields::Text, msg); 
      static_cast<implementation*>(this)->send_message(_session_reject);
      static_cast<implementation*>(this)->disconnect();
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
    atrfix::session_reject _session_reject;
    seqno_store _send_seqno;
    seqno_store _recv_seqno;
    logger& _logger; 
  };

}
