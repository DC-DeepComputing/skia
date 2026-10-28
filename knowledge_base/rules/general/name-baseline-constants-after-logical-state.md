---
name: skia/name-baseline-constants-after-logical-state
description: When declaring baseline reference constants or masks for encoded or biased comparisons, name them after the evaluated logical state rather than encoding mechanics
rubric: readability
severity: nit
---

**Rule.** When declaring baseline reference constants or masks used to test
encoded, biased, or offset representations (such as offset-binary SWAR winding),
name the identifier after the evaluated logical condition or state (e.g.,
emptyBytes or emptySWARMask) rather than the underlying mathematical encoding
(e.g., biasBytes). Ensure identifier names match the semantic terminology used
in accompanying comments and documentation.

**Why.** Naming baseline constants after mathematical encoding mechanics (such
as bias offsets) obscures the condition being checked in comparisons and
diverges from accompanying explanatory comments. Naming constants after the
logical state (such as empty or uncovered) makes downstream comparison
expressions self-describing and clarifies algorithmic intent.

## Examples

Bad:

```cpp
// For the Non-Zero case, we compare subsample winding against the empty SWAR mask.
const Vec8 biasBytes(0x80);
Vec8 cmp = (vecBytes != biasBytes);
```

Good:

```cpp
// For the Non-Zero case, we compare subsample winding against the empty SWAR mask.
const Vec8 emptyBytes(0x80);
Vec8 cmp = (vecBytes != emptyBytes);
```
