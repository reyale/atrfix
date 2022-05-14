#pragma once


namespace atrfix {

namespace consts {

  constexpr char EXAMPLE_FOOTER[] = "10=000\x001";

  constexpr size_t FOOTER_LEN = sizeof(EXAMPLE_FOOTER) - 1;

  constexpr char FIELD_DELIM = '\x001';

}

}
