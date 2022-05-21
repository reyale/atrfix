# atrfix 

lightweight header only FIX engine

There is a suprising lack of options in the open source market for this.  QuickFIX is kind of standard but it's dog slow and its data-dictionary xml model is ridiculous.

## goals

- header only
- minimal dependencies
- easily integrated in to reactor or proactor frameworks
- examples provided
- fast

## repeating fields

Q: Where are they?

A: they're overrated - render them yourself using the raw `set_field` functions in the correct order, or inherit from the message class and implement what you need there

Note: I know they're REALLY important in FICC products and some complex options, I still don't care.

## dependencies

Today we include fmtlib for numerics to strings.  Eventually c++ standard will allow `#include <format>` in which case this is no longer an issue.

## build

There's cmake.  It's just there if you want to build examples.  The library is header only, you dont need the provided build system.  FMTLIB is in header-only mode.

## examples

You can build them with cmake if that's your thing.   
