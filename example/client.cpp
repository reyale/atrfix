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

class example_session;

using logger = atrfix::stdout_logger;
using parent_session = atrfix::session<atrfix::default_clock, atrfix::inmemory_seqnum_store, logger, example_session>;

class example_session : public parent_session { 
public:
  example_session(boost::asio::io_service& io, const std::string & host, const std::string & port) 
    : parent_session("SENDERCOMP", "TARGETCOMP", _logger),  _io_service(io), _socket(io), _main_timer(io), _host(host), _port(std::stoi(port)), 
       _read_buffer(1024) {

    schedule_maintenance();
  }

  void connect() { 
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(_host.c_str()), _port);

    _socket.async_connect(endpoint, [this](boost::system::error_code ec) {
      if(ec) {
        _logger.log("{}", "failed to connect"); 
        return;
      }

      _connected = true;
      send_logon(); 
      schedule_read(); 
    });
  }

  void disconnect() {
    _connected = false;
    _logged_in = false;
    _socket.close();
    _read_buffer.reset();
  }

  void on_message(const char* buffer, size_t len) {
    _read_buffer.mark_read(len);
    _logger.log("< {}", render_fix_visible(std::string_view(buffer, len)));
  }

  template < typename Message >
  void send_message(Message & msg) {
    auto now = _clock.current_time();
    auto time = std::chrono::system_clock::to_time_t(now);
    const auto& result = msg.render(_send_seqno.current(), time);

    _logger.log("> {}", render_fix_visible(sendv(result))); 

    prime_buffer(result, _send_buffer);
    boost::system::error_code ec;
    boost::asio::write(_socket, _send_buffer, ec); //blocking send
    if(ec) {
      _logger.log("{}", "failed to send message, disconnecting");
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
      if(ec) //in practice only handles when io-service destroys and a timer is scheduled 
        return;

      maintain_connection();
      schedule_maintenance();
    });
  }

  void schedule_read() {
    auto [buffer, size] = _read_buffer.write_loc();
    _socket.async_read_some(boost::asio::buffer(buffer, size), [this](const boost::system::error_code& ec, size_t bytes_read) { 
      if(ec) {
        disconnect();
        return;
      }
    
      _read_buffer.mark_write(bytes_read);
      auto [read_buffer, size] = _read_buffer.read_loc();
      handle_read(read_buffer, size);
      _read_buffer.compact(); 
      schedule_read(); 
    });
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


static bool running;

void sig_handler(int signum) {
  std::cout << "caught signal: " << signal << " shutting down" << std::endl;
  running = false;
}

int main(int argc, char* argv[])
{
  signal(SIGINT, sig_handler);

  if (argc != 3) {
    std::cerr << "Usage: <host> <port>\n";
    return 1;
  }

  boost::asio::io_service io_service;

  example_session session(io_service, argv[1], argv[2]);

  running = true;
  while(running)
    io_service.run_for(std::chrono::seconds(1));

  return 0;
}
