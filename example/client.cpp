#include <cstdlib>
#include <array>
#include <iostream>

#include <boost/asio.hpp>

#include "atrfix/session.h"
#include "atrfix/message.h"
#include "atrfix/consts.h"


using boost::asio::ip::tcp;

class example_session : public atrfix::session<atrfix::default_clock, example_session> {
public:
  example_session(boost::asio::io_service& io, const std::string & host, const std::string & port) 
    :  _io_service(io), _socket(io), _main_timer(io), _host(host), _port(std::stoi(port)), _logon_msg("8=FIX.4.4", "SENDERCOMP", "TARGETCOMP") {
    schedule_maintenance();
  }

  void connect() { 
    std::cout << "try to connect: " << _host << ":" << _port << std::endl;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(_host.c_str()), _port);

    _socket.async_connect(endpoint, [this](boost::system::error_code ec) {
      if(ec) {
        std::cout << "failed to connect" << std::endl;
        return;
      }

      //schedule read
    });
  }

  void disconnect() { }
  void send_logon() { 
    if(!_logged_in)
      send_message(_logon_msg);
  }

  void on_message(const char* buffer, size_t len) {
  }

protected:
  template < typename Message >
  void send_message(Message & msg) {
    auto now = _clock.current_time();
    auto time = std::chrono::system_clock::to_time_t(now);
    const auto& result = msg.render(_send_seqno++, time);      
    prime_buffer(result);
    boost::asio::write(_socket, _send_buffer); //blocking send
  }

  void prime_buffer(const atrfix::ioresult & result) {
    auto& tosend = result.iov;
    //boost asio doesn't want raw iov because stupid
    _send_buffer[0] = boost::asio::buffer(tosend[0].iov_base, tosend[0].iov_len);
    _send_buffer[1] = boost::asio::buffer(tosend[1].iov_base, tosend[1].iov_len);
    _send_buffer[2] = boost::asio::buffer(tosend[2].iov_base, tosend[2].iov_len);
  }

  void schedule_maintenance() {
    //maintenance timer for session  
    _main_timer.expires_from_now(boost::posix_time::seconds(15)); 
    _main_timer.async_wait([this](const boost::system::error_code& ec) {
      if(ec)
        return;

      maintain_connection();
      schedule_maintenance();
    });
  }

  boost::asio::io_service& _io_service;
  tcp::socket _socket;
  std::string _host;
  int _port;
  boost::asio::deadline_timer _main_timer;
  atrfix::logon _logon_msg;
  unsigned int _send_seqno = atrfix::consts::STARTING_SEQNO;
  unsigned int _expected_recv_seqno = atrfix::consts::STARTING_SEQNO;
  std::array<boost::asio::const_buffer, 3> _send_buffer;
};


static bool running;

void sig_handler(int signum) {
  std::cout << "caught signal: " << signal << " shutting down" << std::endl;
  running = false;
}

int main(int argc, char* argv[])
{
  signal(SIGINT, sig_handler);

  try {
    if (argc != 3)
    {
      std::cerr << "Usage: <host> <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    example_session session(io_service, argv[1], argv[2]);

    running = true;
    while(running)
      io_service.run_for(std::chrono::seconds(1));

  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl; 
  }

  return 0;
}
