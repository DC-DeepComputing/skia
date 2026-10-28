---
name: skia/core/decode-bitwise-inverted-indices-before-offset
description: When adjusting a bitwise-inverted encoded index by a container offset, decode the index first before adding the offset rather than subtracting inside the inversion
rubric: correctness
severity: should_fix
---

**Rule.** When offsetting an index that uses bitwise-NOT encoding (such as
`~index` used for sentinel states or flags), decode the index first with
`~index` and add the container offset directly. Do not perform subtraction
inside the bitwise complement (`~(index - offset)`), which obscures intent and
relies on counterintuitive negative two's-complement arithmetic.

**Why.** Performing arithmetic inside a bitwise inversion obscures intent and
increases the risk of sign or off-by-one errors. Decoding the non-negative index
first before adding positive container offsets aligns with standard container
indexing models and makes the calculation immediately clear.

## Examples

Bad:

```cpp
// Subtraction inside bitwise complement relies on counterintuitive negative two's complement
fLastMoveIndex = ~(src.fLastMoveToIndex - this->countPoints());
```

Good:

```cpp
// Decode to positive index first, then add the non-negative container offset
fLastMoveIndex = ~src.fLastMoveToIndex + this->countPoints();
```
