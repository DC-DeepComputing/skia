---
name: skia/promote-narrow-integers-in-sum-bounds-checks
description: When validating that sums of narrow integer values fit within smaller integer limits, promote operands to wider types instead of writing subtractive checks
rubric: readability
severity: should_fix
---

**Rule.** When validating that the sum of narrow integer operands (such as
uint16_t or uint8_t) fits within a narrow type limit, promote operands to a
wider integer type (such as uint32_t or size_t) to perform a single direct
addition and comparison against std::numeric_limits<T>::max() or maximum
constant. Avoid writing convoluted multi-step subtractive checks (e.g., max - a
>= b + 1) to circumvent intermediate overflow.

**Why.** In narrow integer arithmetic, intermediate sums can overflow the narrow
type's range, leading authors to write inverted or multi-step subtractive
inequality checks that are difficult to read and verify. Promoting narrow
operands to a wider type like uint32_t where intermediate overflow cannot occur
allows checking the exact sum against the limit in a single, readable assertion.

## Examples

Bad:

```cpp
// Bad: Convoluted subtractive checks to avoid intermediate uint16_t overflow
SkASSERT_RELEASE(std::numeric_limits<uint16_t>::max() - 1 >= extraData32Cnt);
SkASSERT_RELEASE(std::numeric_limits<uint16_t>::max() - innerData32Cnt >=
                 extraData32Cnt + 1);
```

Good:

```cpp
// Good: Promoting operands to uint32_t simplifies the bounds check to a single sum
SkASSERT_RELEASE((uint32_t)extraData32Cnt + (uint32_t)innerData32Cnt + 1 <=
                 (uint32_t)std::numeric_limits<uint16_t>::max());
```
