#pragma once

#include <chrono>


namespace atrfix {

  template < typename T >
  concept subtractable = requires(T a, T b) {
    { a - b };
  };

  template <typename C>
  concept clock_interface = requires(C clock) {
    typename C::timestamp;
    typename C::timedelta;

    requires subtractable<typename C::timestamp>; 
    { clock.current_time() } -> typename C::timestamp;
    { clock.from_seconds(int()) } -> typename C::timedelta;
  };

  class default_clock_interface {
  public:
    using clock = std::chrono::system_clock;
    using timestamp = std::chrono::time_point<clock>;
    using timedelta = std::chrono::nanoseconds; 

    default_clock_interface() { }
    timestamp current_time() { return clock::now(); }

    timedelta from_seconds(int secs) { return std::chrono::seconds(secs); }
  };

}
