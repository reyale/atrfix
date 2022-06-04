#pragma once

#include <cstdlib>
#include <array>
#include <iostream>
#include <strings.h>

#include <boost/asio.hpp>

#include "atrfix/session.h"
#include "atrfix/message.h"
#include "atrfix/consts.h"
#include "atrfix/rwbuffer.h"
#include "example_utils.h"
#include "logger.h"


using boost::asio::ip::tcp;
using namespace example;

class asio_client;

using logger = atrfix::stdout_logger;
using parent_session = atrfix::session<atrfix::default_clock, atrfix::inmemory_seqnum_store, logger, asio_client>;

class asio_client : public parent_session { 
public:
  asio_client(boost::asio::io_service& io, const std::string & host, const std::string & port, const std::string & sendercomp, const std::string & targetcomp) 
    : parent_session(sendercomp, targetcomp, _logger),  _io_service(io), _socket(io), _main_timer(io), _host(host), _port(std::stoi(port)), 
       _read_buffer(1024) {

    schedule_maintenance();
  }

  void connect() { 
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(_host.c_str()), _port);
    _socket.async_connect(endpoint, [this](boost::system::error_code ec) { handle_connect(ec); });
  }

  void disconnect() {
    _logger.log("{}\n", "disconnected");
    _connected = false;
    _logged_in = false;
    _socket.close();
    _read_buffer.reset();
  }

  void on_message(const char* buffer, size_t len) {
    _read_buffer.mark_read(len);
    _logger.log("< {}\n", render_fix_visible(std::string_view(buffer, len)));
  }

  template < typename Message >
  void send_message(Message & msg) {
    auto now = _clock.current_time();
    auto time = std::chrono::system_clock::to_time_t(now);
    const auto& result = msg.render(_send_seqno.current(), time);

    _logger.log("> {}\n", render_fix_visible(sendv(result))); 

    prime_buffer(result, _send_buffer);
    boost::system::error_code ec;
    boost::asio::write(_socket, _send_buffer, ec); //blocking send
    if(ec) {
      _logger.log("{}\n", "failed to send message, disconnecting");
      disconnect();
      return;
    }

    _send_seqno.increment();
    _last_sent_msg = _clock.current_time();
  }

protected:
  void schedule_maintenance() {
    //maintenance timer for session  
    _main_timer.expires_from_now(boost::posix_time::seconds(15)); 
    _main_timer.async_wait([this](const boost::system::error_code& ec) {
      if(ec) { //in practice only happens when io-service destroys and a timer is scheduled 
        _logger.log("timer failure msg={}", ec.message());
        return;
      }

      maintain_connection();
      schedule_maintenance();
    });
  }

  void schedule_read() {
    auto [buffer, size] = _read_buffer.write_loc();
    _socket.async_read_some(boost::asio::buffer(buffer, size), 
            [this](const boost::system::error_code& ec, size_t bytes_read) {
                read_some(ec, bytes_read);
              }); 
  }

  void read_some(const boost::system::error_code ec, size_t bytes_read) {
    if(ec) {
      _logger.log("{} msg={}\n", "failure to read, disconnecting", ec.message());
      disconnect();
      return;
    }

    _read_buffer.mark_write(bytes_read);
    auto [read_buffer, size] = _read_buffer.read_loc();
    handle_read(read_buffer, size);
    _read_buffer.compact();
    schedule_read();
  }

  void handle_connect(boost::system::error_code ec) {
    if(ec) {
      _logger.log("{}\n", "failed to connect");
      return;
    }

    _connected = true;
    send_logon();
    schedule_read();
  }

  boost::asio::io_service& _io_service;
  tcp::socket _socket;
  std::string _host;
  int _port;
  boost::asio::deadline_timer _main_timer;

  asio_send_buffer _send_buffer;
  atrfix::rwbuffer _read_buffer;
  logger _logger;
};
