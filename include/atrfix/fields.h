#pragma once


namespace atrfix {

namespace fields {

  constexpr char BeginString[] = "8";
  constexpr char BodyLength[] = "9";
  constexpr char ClOrdID[] = "11";
  constexpr char EndSeqNo[] = "16";
  constexpr char HandInst[] = "21";
  constexpr char MsgType[] = "35";
  constexpr char SenderCompID[] = "49";
  constexpr char TargetCompID[] = "56";
  constexpr char MsgSeqNum[] = "34";
  constexpr char NewSeqNo[] = "36";
  constexpr char OrderQty[] = "38";
  constexpr char OrdType[] = "40";
  constexpr char Price[] = "44";
  constexpr char RefSeqNum[] = "45";
  constexpr char SendingTime[] = "52";
  constexpr char Side[] = "54";
  constexpr char Symbol[] = "55";
  constexpr char Text[] = "58";
  constexpr char TransactTime[] = "60";
  constexpr char EncryptMethod[] = "98";
  constexpr char HeartBtInt[] = "108";
  constexpr char GapFillFlag[] = "123";

  namespace ints { //since fix is strings these are less useful, but can be useful when parsing
    constexpr int ClOrdID = 11;
    constexpr int OrdStatus = 39;
    constexpr int ExecType = 150;
  }

}

}
