#pragma once

  namespace example {

    using asio_send_buffer = std::array<boost::asio::const_buffer, 3>;

    std::string
    sendv(const atrfix::ioresult & result) {
      return fmt::format("{}{}{}", std::string_view((char*)result.iov[0].iov_base, result.iov[0].iov_len),
                               std::string_view((char*)result.iov[1].iov_base, result.iov[1].iov_len),
                               std::string_view((char*)result.iov[2].iov_base, result.iov[2].iov_len));
    }

    void log_fix(auto && msg) {
      for(size_t i = 0; i < msg.size(); ++i) {
        if(msg[i] == '\001')
          std::cout << "|";
        else
          std::cout << msg[i];
      }
      std::cout << std::endl;
    }

    void prime_buffer(const atrfix::ioresult & result, asio_send_buffer & send_buffer) {
      auto& tosend = result.iov;

      send_buffer[0] = boost::asio::buffer(tosend[0].iov_base, tosend[0].iov_len);
      send_buffer[1] = boost::asio::buffer(tosend[1].iov_base, tosend[1].iov_len);
      send_buffer[2] = boost::asio::buffer(tosend[2].iov_base, tosend[2].iov_len);
    }
  }
