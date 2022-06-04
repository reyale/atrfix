#include <cstdlib>
#include <iostream>

#include <boost/asio.hpp>

#include "asio_client.h"


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

  //whatever is relevant
  const std::string sender("SENDERCOMP");
  const std::string target("TARGETCOMP");

  asio_client client(io_service, argv[1], argv[2], sender, target);

  running = true;
  while(running)
    io_service.run_for(std::chrono::seconds(1));

  return 0;
}
