---
name: skia/api/prefer-optional-over-sentinel-values
description: When defining optional configuration fields, parameters, local variables, or fallible lookups, use std::optional<T> instead of in-band sentinels like -1 or 0
rubric: readability
severity: should_fix
---

**Rule.** When defining configuration fields, parameters, properties, or local
variables tracking conditionally set values, use std::optional<T> instead of
magic sentinel values (such as -1, 0, or SIZE_MAX) to represent absence or
fallback to default behavior. Similarly, when defining query, accessor, or
lookup methods where a requested element may be missing or an index may be out
of bounds, return std::optional<T> rather than in-band sentinel values (such as
(0, 0) or default-constructed objects).

**Why.** In-band sentinel values (such as 0, -1, magic numbers, (0, 0), or
default-constructed instances) are ambiguous because they can collide with valid
data (e.g. coordinate origins or deliberate 0/-1 configurations) and force
callers to rely on magic constants. Using std::optional<T> explicitly models
presence or absence at the type level, making conditional assignment, missing
lookups, and default fallback resolution (e.g. via .value_or()) clearer, safer,
and less error-prone.

## Examples

Bad:

```cpp
// Bad: Sentinel value (-1) used to denote unset/default behavior.
class SkFontArguments {
public:
    SkFontArguments& setSimulationsFlags(int flags);
    int simulationsFlags() const { return fSimulationsFlags; }
private:
    int fSimulationsFlags = -1; // -1 denotes "unspecified / use default"
};

// Bad: Using 0 sentinel for conditionally populated local value.
int requestedWidth = 0;
for (size_t i = 0; i < requestAxisCount; ++i) {
    if (tag == wdth) {
        requestedWidth = SkFontDescriptor::SkFontStyleWidthForWidthAxisValue(value);
    }
}
int width = requestedWidth != 0 ? requestedWidth : style.width();

// Bad: In-band coordinate sentinel (0, 0) returned on out-of-bounds lookup.
SkPoint getPoint(int index) const {
    if ((unsigned)index < (unsigned)fPts.size()) {
        return fPts.at(index);
    }
    return SkPoint::Make(0, 0); // Collides with a valid origin point!
}
```

Good:

```cpp
// Good: std::optional distinguishes unset/default from a specified value.
class SkFontArguments {
public:
    SkFontArguments& setSyntheticBold(std::optional<bool> bold);
    std::optional<bool> syntheticBold() const { return fSyntheticBold; }
private:
    std::optional<bool> fSyntheticBold; // nullopt denotes "unspecified / use default"
};

// Good: std::optional models presence and enables clean value_or() fallback.
std::optional<int> requestedWidth;
for (size_t i = 0; i < requestAxisCount; ++i) {
    if (tag == wdth) {
        requestedWidth = SkFontDescriptor::SkFontStyleWidthForWidthAxisValue(value);
    }
}
int width = requestedWidth.value_or(style.width());

// Good: std::optional<SkPoint> explicitly signals missing element / invalid index.
std::optional<SkPoint> getPoint(int index) const {
    if ((unsigned)index < (unsigned)fPts.size()) {
        return fPts.at(index);
    }
    return std::nullopt;
}
```
