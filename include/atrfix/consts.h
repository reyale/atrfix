#pragma once


namespace atrfix {

namespace consts {

  constexpr int CHECKSUM_MOD = 256;

  constexpr char EXAMPLE_FOOTER[] = "10=000\001";

  constexpr size_t FOOTER_LEN = sizeof(EXAMPLE_FOOTER) - 1;

  constexpr char FIELD_DELIM = '\001';

  namespace beginstrs {
    constexpr char FIX44[] = "8=FIX.4.4\001";
  }

  static unsigned int STARTING_SEQNO = 1;
}

}
