---
name: skia/modules/zero-based-offset-directional-range-iteration
description: When iterating a range forward or in reverse depending on text direction, loop over a zero-based offset and compute symmetric start/end indices
rubric: readability
severity: should_fix
---

**Rule.** When iterating over an index range in either forward or reverse order
depending on direction (such as LTR vs. RTL text runs), iterate over a
zero-based offset `[0, count)` and compute symmetric indices (`start + index`
for forward, `end - index - 1` for reverse) rather than looping over absolute
forward indices and mapping to reverse positions.

**Why.** Iterating over a zero-based offset decouples the traversal step count
from the indexing direction, making forward and reverse element access symmetric
and easy to follow. Iterating over absolute forward indices and converting them
to reverse positions obscures intent and requires error-prone arithmetic.

## Examples

Bad:

```cpp
for (BlockIndex index = fBlockRange.start; index <= fBlockRange.end; ++index) {
    if (index < fBlockRange.end) {
        auto block = fOwner->styles().begin() +
            (run->leftToRight() ? index : fBlockRange.end - index + fBlockRange.start - 1);
    }
}
```

Good:

```cpp
const BlockIndex blockRangeSize = fBlockRange.end - fBlockRange.start;
for (BlockIndex index = 0; index <= blockRangeSize; ++index) {
    if (index < blockRangeSize) {
        auto block = fOwner->styles().begin() +
            (run->leftToRight() ? fBlockRange.start + index : fBlockRange.end - index - 1);
    }
}
```
