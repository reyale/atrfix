#include <cstdlib>
#include <iostream>
#include <unordered_map>

#include <boost/asio.hpp>

#include "atrfix/consts.h"
#include "atrfix/fields.h"
#include "asio_client.h"


class order_sender_client : public asio_client {
public:
  order_sender_client(boost::asio::io_service & io, const std::string & host, const std::string & port, const std::string & sendercomp, const std::string & targetcomp) 
    : asio_client(io, host, port, sendercomp, targetcomp), 
    _order_timer(io), _order_message(atrfix::consts::beginstrs::FIX44, sendercomp, targetcomp) {

    schedule_order_timer();
  }

protected:

  void schedule_order_timer() {
    _order_timer.expires_from_now(boost::posix_time::seconds(10));
    _order_timer.async_wait([this](const boost::system::error_code & ec) {
                              if(ec) {
                                _logger.log("order timer failure msg={}", ec.message());
                                return; 
                              }

                              maintain_orders();
                              schedule_order_timer();
                            });
  }

  void maintain_orders() {

    //clean out complete orders
    std::erase_if(_order_state, [](const auto& item) {
        auto const& [key, value] = item;
          //filled, done for day, cancelled, rejected, stopped
          if(value == "8" || value == "10" || value == "5" || value == "2" || value == "7")
            return true;

          return false;
        }); 

    if(!session_ready())
      return;

    //if there is nothing live send a new order 
    if(_order_state.size() < 1) {
      static bool flip = 1;
      _order_message.reset();

      //send order
      _order_message.set_field(atrfix::fields::ClOrdID, _clordid++);
      _order_message.set_field(atrfix::fields::HandInst, "1");
      _order_message.set_field(atrfix::fields::OrdType, "2"); //limit
      _order_message.set_field(atrfix::fields::OrderQty, 100); 
      if(flip)
        _order_message.set_field(atrfix::fields::Price, 100.0); //buy low 
      else
        _order_message.set_field(atrfix::fields::Price, 101.0); //sell high

      if(flip)
        _order_message.set_field(atrfix::fields::Side, "1"); //buy
      else
        _order_message.set_field(atrfix::fields::Side, "2"); //sell

      _order_message.set_field(atrfix::fields::Symbol, "IBM");
      auto time = std::chrono::system_clock::to_time_t(_clock.current_time());
      _order_message.set_field(atrfix::fields::TransactTime, time);
      send_message(_order_message); 
      flip = !flip; 
    } 
  }

  void on_application_message(char msgtype, const char* buffer, size_t len) {
    if(msgtype != atrfix::msgtype::ExecutionReport)
      return;

    /*simple non-performant parse to a hash as an example
     * this function is not meant to be proper order state handling, it's just an example
     */
    
    std::unordered_map<int, std::string> parse_results;
    atrfix::hash_parse(buffer, len, parse_results);
    
    auto order_status = parse_results.find(atrfix::fields::ints::OrdStatus);
    if(order_status == parse_results.end())
      return; //could send session level reject for this, but no reason in test client

    auto clordid_iter = parse_results.find(atrfix::fields::ints::ClOrdID);
    if(clordid_iter == parse_results.end())
      return; //could send session level reject for this, but no reason in test client


    int clordid = atrfix::parse((*clordid_iter).second);
    if(clordid == std::numeric_limits<int>::min())
      return;

    std::cout << "on_application_message: msgtype=" << msgtype << " clordid=" << clordid << " newstate=" << (*order_status).second << std::endl;

    _order_state[clordid] = (*order_status).second; //just adopt the latest
  }

  boost::asio::deadline_timer _order_timer;
  std::unordered_map<int, std::string> _order_state;
  atrfix::new_order_single _order_message;
  int _clordid = 1; //should consider making it unique per date 
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

  //whatever is relevant
  const std::string sender("SENDERCOMP");
  const std::string target("TARGETCOMP");

  order_sender_client client(io_service, argv[1], argv[2], sender, target);

  running = true;
  while(running)
    io_service.run_for(std::chrono::seconds(1));

  return 0;
}
