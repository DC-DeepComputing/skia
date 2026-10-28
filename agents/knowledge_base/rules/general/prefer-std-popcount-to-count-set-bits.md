---
name: skia/prefer-std-popcount-to-count-set-bits
description: When counting set bits in an integer or bitmask, use std::popcount or SkPopCount instead of manual bit-shifting loops or hand-rolled bitwise arithmetic
rubric: readability
severity: should_fix
---

**Rule.** When counting set bits in an integer, bitmask, or difference mask, use
std::popcount (from <bit>) or Skia's SkPopCount helper (from SkMathPriv.h). Do
not open-code manual bit-shifting loops (such as looping over bit indices and
testing (x >> b) & 1) or hand-rolled bitwise counting tricks.

**Why.** Standard bit-manipulation utilities clearly convey programmer intent,
eliminate loop and masking boilerplate, and map directly to dedicated hardware
popcount CPU instructions rather than executing multi-iteration branching loops
or multi-step bitwise arithmetic.

## Examples

Bad:

```cpp
uint32_t bitErrors = 0;
for (int b = 0; b < 8; ++b) {
    bitErrors += (diff >> b) & 1;
}
```

Good:

```cpp
uint32_t bitErrors = std::popcount(diff);
```
