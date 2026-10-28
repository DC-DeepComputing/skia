---
name: skia/prefer-std-partition-over-std-remove-if
description: When filtering container elements where relative order is not preserved, prefer std::partition over std::remove_if to avoid redundant moves and shifting
rubric: safety
severity: should_fix
---

**Rule.** Prefer std::partition over std::remove_if when filtering sequence
container elements if relative ordering does not need to be preserved (such as
immediately before std::sort). Invert the predicate so retained elements are
partitioned to the front of the container.

**Why.** std::remove_if preserves the relative order of elements, shifting all
subsequent retained elements forward for each removal and incurring up to O(N)
moves or copies. When relative ordering does not matter, std::partition swaps
elements across the partition boundary instead of shifting, performing
significantly fewer moves.

## Examples

Bad:

```cpp
auto isZeroSegment = [](const Segment& segment) {
    return segment.upper() == segment.lower();
};
// Unnecessarily shifts all subsequent elements forward to preserve relative order
const auto zeroSegments = std::remove_if(segments.begin(), segments.end(), isZeroSegment);
std::sort(segments.begin(), zeroSegments);
```

Good:

```cpp
auto isNonZeroSegment = [](const Segment& segment) {
    return segment.upper() != segment.lower();
};
// Inverts predicate and swaps elements without shifting
const auto zeroSegments = std::partition(segments.begin(), segments.end(), isNonZeroSegment);
std::sort(segments.begin(), zeroSegments);
```
