---
name: skia/api/name-level-enums-by-magnitude
description: When defining enums that represent a level, scale, or degree, name enumerators after the magnitude itself rather than secondary tradeoffs like speed
rubric: readability
severity: nit
---

**Rule.** Name enumerators of an enum representing a level, scale, or degree
(such as `CompressionLevel`, `QualityLevel`, or `OptimizationLevel`) after the
magnitude or level itself (e.g., `kLow`, `kMedium`, `kHigh`) rather than
secondary characteristics or operational tradeoffs like execution speed (e.g.,
`kFast`, `kSlow`).

**Why.** When an enum models a degree or intensity scale, naming enumerators
after side-effect tradeoffs like speed inverts the semantic relationship and
creates ambiguity about which direction represents greater intensity. Naming
variants directly by magnitude keeps the enumerators aligned with the enum
type's dimension.

## Examples

Bad:

```cpp
enum class CompressionLevel : uint8_t {
    kFast,
    kAverage,
    kSlow,
};
```

Good:

```cpp
enum class CompressionLevel : uint8_t {
    kLow,
    kMedium,
    kHigh,
};
```
