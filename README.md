# atrfix 

lightweight header only FIX engine

There is a suprising lack of options in the open source market for this.  QuickFIX is kind of standard but it's dog slow and its data-dictionary xml model is ridiculous.

## goals

- header only
- minimal dependencies
- easily integrated in to reactor or proactor frameworks
- examples provided
- fast

## dependencies

Today we include fmtlib for numerics to strings.  Eventually c++ standard will allow `#include <format>` in which case this is no longer an issue.
