#pragma once 

#include <string.h>


namespace atrfix {

  class rwbuffer {
  public:
    rwbuffer(size_t capacity) : _write_loc(0), _read_loc(0), _capacity(capacity) { _buffer = new char[capacity]; }

    bool compact() {
      if(_write_loc == 0)
        return false;

      if(_write_loc > _read_loc) {
        ::memmove((void*)_buffer, (void*)(_buffer + _read_loc), _write_loc - _read_loc);
        _write_loc = _write_loc - _read_loc;
      } else {
        _write_loc = 0;
      }

      _read_loc = 0;
      return true;
    }

    auto write_size() {
      return _capacity - _write_loc;
    }

    auto write_loc() {
      return std::make_tuple(_buffer + _write_loc, _capacity - _write_loc);
    }

    auto read_loc() {
      return std::make_tuple( _buffer + _read_loc, _write_loc - _read_loc);
    }

    auto mark_read(size_t amount) {
      _read_loc += amount;
    }

    auto mark_write(size_t amount) {
      _write_loc += amount;
    }

    void reset() {
      _write_loc = 0;
      _read_loc = 0;
    }

  protected:
    size_t _write_loc;
    size_t _read_loc;
    size_t _capacity;
    char* _buffer;
  }; 

}
