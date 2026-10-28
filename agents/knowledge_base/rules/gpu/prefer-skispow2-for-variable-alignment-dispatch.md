---
name: skia/gpu/prefer-skispow2-for-variable-alignment-dispatch
description: When aligning offsets to a variable alignment requirement, check SkIsPow2 to dispatch to SkAlignTo instead of testing equality against a minimum alignment
rubric: safety
severity: should_fix
---

**Rule.** When aligning buffer offsets or sizes to a variable alignment
requirement, check `SkIsPow2(req)` rather than comparing against a known
power-of-two minimum (`req == min`) to dispatch to `SkAlignTo` instead of
`SkAlignNonPow2`. In addition, do not wrap the requirement in `std::max(req,
min)` when divisibility or ordering is already established.

**Why.** `SkAlignTo` uses efficient bitwise masking for power-of-two alignments,
whereas `SkAlignNonPow2` relies on costly integer division and multiplication.
Checking `SkIsPow2(req)` ensures that larger power-of-two multiples (such as 8,
16, or 64) take the fast bitwise path rather than unnecessarily falling back to
`SkAlignNonPow2`.

## Examples

Bad:

```cpp
// Flawed: testing equality against min misses larger power-of-two alignments.
if (req != minAlignment) {
    offset = SkTo<uint32_t>(SkAlignNonPow2(offset, std::max(req, minAlignment)));
} else {
    offset = SkTo<uint32_t>(SkAlignTo(offset, minAlignment));
}
```

Good:

```cpp
// Correct: SkIsPow2 routes all power-of-two alignments to fast bitwise masking.
if (SkIsPow2(req)) {
    offset = SkTo<uint32_t>(SkAlignTo(offset, req));
} else {
    offset = SkTo<uint32_t>(SkAlignNonPow2(offset, req));
}
```
