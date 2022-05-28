#pragma once

#include "atrfix/consts.h"


namespace atrfix {

  template < typename S >
  concept seqnum_store_interface = requires(S s) {

    { s.increment() }; 
    { s.current() } -> seqno;
    { s.set(seqno()) };

  };

  class inmemory_seqnum_store {
  public:
    inmemory_seqnum_store() : num(consts::STARTING_SEQNO) { }
    void set(seqno n) { num = n; }
    void increment() { ++num; }
    seqno current() const { return num; }

  private:
    seqno num;
  };

}
