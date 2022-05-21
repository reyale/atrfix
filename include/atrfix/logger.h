#pragma once

#include <fstream>

#include "fmt/format.h"


namespace atrfix {

  class file_logger {
  public:
    file_logger(const std::string & fname) {
      _file.open(fname);
    }
      
    ~file_logger() {
      _file.close();
    }

    template < class ... Args >
    size_t log(const char* fmt, Args&&...args) {
      auto res = fmt::format(fmt, args...);
      _file.write(res.data(), res.size());
      return res.size();
    }

  protected:
    std::ofstream _file;
  };
}
