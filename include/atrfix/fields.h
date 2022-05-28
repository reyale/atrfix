#pragma once


namespace atrfix {

namespace fields {

  constexpr char BeginString[] = "8";
  constexpr char BodyLength[] = "9";
  constexpr char EndSeqNo[] = "16";
  constexpr char MsgType[] = "35";
  constexpr char SenderCompID[] = "49";
  constexpr char TargetCompID[] = "56";
  constexpr char MsgSeqNum[] = "34";
  constexpr char NewSeqNo[] = "36";
  constexpr char SendingTime[] = "52";
  constexpr char Price[] = "44";
  constexpr char EncryptMethod[] = "98";
  constexpr char HeartBtInt[] = "108";
  constexpr char GapFillFlag[] = "123";
}

}
