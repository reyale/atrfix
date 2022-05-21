#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>

#include "atrfix/session.h"


using boost::asio::ip::tcp;

class example_session : public atrfix::session<atrfix::default_clock, example_session> {
public:
  example_session(boost::asio::io_service& io, const std::string & host, const std::string & port) :  _io_service(io), _socket(io), _main_timer(io) {
    schedule_maintenance();
  }

  void connect() { 
    std::cout << "try to connect" << std::endl;
    tcp::resolver resolver(_io_service);
    auto endpoint_iterator = resolver.resolve({_host, _port});
    boost::asio::async_connect(_socket, endpoint_iterator, [this](boost::system::error_code ec, tcp::endpoint) {
      if(ec) {
        std::cout << "failed to connect" << std::endl;
        return;
      }

      //schedule read
    });
  }

  void disconnect() { }
  void send_logon() { }

protected:
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
  std::string _port;
  boost::asio::deadline_timer _main_timer;
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

    example_session session(io_service, argv[0], argv[1]);

    running = true;
    while(running)
      io_service.run_for(std::chrono::seconds(1));

  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl; 
  }

  return 0;
}
