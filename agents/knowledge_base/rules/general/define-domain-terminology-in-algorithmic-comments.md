---
name: skia/define-domain-terminology-in-algorithmic-comments
description: When documenting complex algorithms or multi-step processing in comments, define domain terms upfront and cite concrete sizing constants alongside conceptual buffer requirements
rubric: readability
severity: nit
---

**Rule.** When documenting algorithms, subsystem architectures, or multi-step
processing in code comments, explicitly define specialized domain terms and
explain the conditions under which edge cases arise (such as leftover items when
dimensions are not divisible by SIMD stride) before describing the steps taken
to handle them. Additionally, when describing temporary storage, scratch
buffers, or memory requirements, state the concrete sizing constant, buffer
limit, or struct field used in practice alongside the conceptual requirement.

**Why.** Algorithmic comments that jump directly into edge-case handling using
colloquial jargon (such as 'tail' pixels) assume readers already understand why
and when that condition occurs, while purely abstract descriptions of buffer
requirements leave readers guessing which concrete constants and data structures
implement the design. Explicitly defining terms and linking conceptual sizing
requirements to concrete constants bridges high-level algorithms with actual
memory layouts and makes complex processing maintainable.

## Examples

Bad:

```cpp
// Bad: Using undefined jargon without establishing conditions, and omitting concrete constants:
// Raster Pipeline typically processes N (4, 8, 16) pixels at a time, in SIMT fashion. To avoid
// reading or writing past the end of any source or destination buffers when we reach the "tail"
// pixels in a row (width < N):
//   1) Source buffers have their tail contents copied to a scratch buffer that is at least N wide.
```

Good:

```cpp
// Good: Defining domain terms/conditions upfront, and stating concrete sizing constants:
// Raster Pipeline typically processes N (4, 8, 16) pixels at a time, in SIMT fashion. If the
// number of pixels in a row isn't evenly divisible by N, there will be leftover pixels; this is
// called the "tail". To avoid reading or writing past the end of any buffers when we reach the tail:
//   1) Source buffers have their tail contents copied to a scratch buffer that is at least N wide.
//      In practice, each scratch buffer uses kMaxScratchPerPatch bytes.
```
