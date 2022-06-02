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

## general use and design

### atrfix::message 

Reusable fix message renderer

1. User can call `reset` to resuse the message structure
2. User calls `render(seqno, time)` to generate the final raw message for the wire

If more complicated message handling, validation, repeating field support, ect is required than the user should extend this class

### atrfix::session 

fix session that is reusable across transport and event framework (boost::asio, libevent, ect)

1. The fix session is maintained via periodic (user scheduled) calls to `maintain_session`
2. Ther user calls `handle_read` and session and transport level symantics are handled 
3. Calls to `handle_read` will call you back with raw messages via `on_message`
4. The user handles final FIX message sending via `send_message` 
5. The user handles transport connection via `connect`
6. the use rhandles transport disconnection via `disconnect`

Since the user is in control of the transport symantics and final sending the rendered messages on the wire this framework could be used to send messaes over things such as:

1. TCP
2. UDP
3. SSL

### parsing

Parsing data from `on_message` is completely left to the user, but there's `parser.h` for utility purposes.

## repeating fields

Q: Where are they?

A: They're overrated - render them yourself using the raw `set_field` functions in the correct order, or inherit from the message class and implement what you need there

Note: I know they're REALLY important in FICC products and some complex options, I still don't care.

## replay and recovery

This engine is written with a client perspective in mind.  If you're an exchange or broker you need to support full replay for your clients, and ask for full resends.  Generally a client will not want to resend orders if they were missed as they're now stale.  As such our policy is:

1. If we send a seqno > than the counterparty expects:  We should get a resend request, we then send a sequence reset to skip ahead (we don't want to resend missed orders).
2. If we send a seqno < counterparty expects:  We should get 35=3 (session level reject) and we disconnect.

1. If we receive a seqno < we expect: TODO: change to disconnect, for now this helps testing  
2. If we receive a seqno > we expect: We send a replay request. NOT IMPLEMENTED YET - we disconnect 

## dependencies

Today we include fmtlib for numerics to strings.  Eventually c++ standard will allow `#include <format>` in which case this is no longer an issue.
boost::asio is needed to compile the example `client.cpp` which is a boost::asio TCP client

## build

There's cmake.  It's just there if you want to build examples.  The library is header only, you dont need the provided build system.  FMTLIB is in header-only mode.

## examples

You can build them with cmake if that's your thing. 

"client.cpp" is a boost::asio TCP FIX client.  

## test

There's `test.cpp` which builds `test`.  This just uses asserts as I didn't want to import a whole unit test framework
