# atrfix 

lightweight header only FIX engine

There is a suprising lack of options in the open-source world for this.  QuickFIX is kind of standard but it's dog slow and its data-dictionary XML model is ridiculous.

## goals

- header only
- minimal dependencies
- easily integrated in to reactor or proactor frameworks like boost:asio, libevent, ect
- seperation of session and message creation/parsing
- fast
- examples provided

## repeating fields

Q: Where are they?

A: they're overrated - render them yourself using the raw `set_field` functions in the correct order, or inherit from the message class and implement what you need there

Note: I know they're REALLY important in FICC products and some complex options, I still don't care.

## replay

This engine is written with a client perspective in mind.  If you're an exchange or broker you need to support full replay for your clients, and ask for full resends.  Generally a client will not want to resend orders if they were missed as they're now stale.  As such our policy is:

1. If we send a seqno > than the counterparty expects:  We should get a resend request, we then send a sequence reset to skip ahead (we don't want to resend missed orders).
2. If we send a seqno < counterparty expects:  We should get 35=3 (session level reject) and we disconnect.

1. If we receive a seqno < we expect: TODO: change to disconnect, for now this helps testing  
2. If we receive a seqno > we expect: We send a replay request. NOT IMPLEMENTED YET - we disconnect 

## dependencies

Today we include fmtlib for numerics to strings.  Eventually c++ standard will allow `#include <format>` in which case this is no longer an issue.

## build

There's cmake.  It's just there if you want to build examples.  The library is header only, you dont need the provided build system.  FMTLIB is in header-only mode.

## examples

You can build them with cmake if that's your thing. 

"client.cpp" is a boost::asio FIX client.  
