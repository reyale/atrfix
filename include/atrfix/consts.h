#pragma once


namespace atrfix {

using seqno = unsigned int;

namespace consts {

  constexpr int CHECKSUM_MOD = 256;

  constexpr char EXAMPLE_FOOTER[] = "10=000\001";

  constexpr size_t FOOTER_LEN = sizeof(EXAMPLE_FOOTER) - 1;

  constexpr char FIELD_DELIM = '\001';

  namespace beginstrs {
    constexpr char FIX44[] = "8=FIX.4.4";
  }

  namespace msgtype { 
    constexpr char Heartbeat = '0';
    constexpr char TestRequest = '1';
    constexpr char ResendRequest = '2';
    constexpr char Reject = '3';
    constexpr char SequenceReset = '4';
    constexpr char Logout = '5';
    constexpr char IndicationofInterest = '6';
    constexpr char ExecutionReport = '8';
    constexpr char OrderCancelReject = '9';
    constexpr char Logon = 'A';
    constexpr char Order = 'D';
    constexpr char OrderCancelRequest = 'F';
    constexpr char OrderCancelReplaceRequest = 'G';
    constexpr char INVALID = '\001';
  }

  static seqno STARTING_SEQNO = 1;
}

}
