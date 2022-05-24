#pragma once

  namespace example {
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
  }
