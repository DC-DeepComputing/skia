---
name: skia/api/document-span-size-mismatch-consistently
description: When documenting APIs taking paired input and output spans (e.g. SkSpan dst and src), state the size-mismatch contract consistently across all sibling functions
rubric: readability
severity: nit
---

**Rule.** When documenting APIs that accept paired input and output spans (e.g.
SkSpan<T> dst and SkSpan<const T> src), document the size-mismatch contract
(such as processing min(dst.size(), src.size()) elements or requiring equal
lengths) consistently across all related functions and overloads in the API
family. Do not specify sizing behavior on only one function while leaving
sibling overloads ambiguous.

**Why.** Omitting the size-mismatch contract on sibling overloads leaves callers
uncertain whether functions require matching lengths or follow the same
truncation/processing policy. Explicit and consistent documentation across
related methods prevents caller bugs and eliminates ambiguity about partial
writes or buffer overruns.

## Examples

Bad:

```cpp
/**
 * @param dst  span where the transformed points are written
 * @param src  span where the points are read from
 *
 * Note: min(dst.size(), src.size()) is the number of points that will be written to dst.
 */
void mapPoints(SkSpan<SkPoint> dst, SkSpan<const SkPoint> src) const;

/**
 * @param dst  span where the transformed points are written
 * @param src  span where the points are read from
 */
void mapHomogeneousPoints(SkSpan<SkPoint3> dst, SkSpan<const SkPoint3> src) const;
```

Good:

```cpp
/**
 * @param dst  span where the transformed points are written
 * @param src  span where the points are read from
 *
 * Note: min(dst.size(), src.size()) is the number of points that will be written to dst.
 */
void mapPoints(SkSpan<SkPoint> dst, SkSpan<const SkPoint> src) const;

/**
 * @param dst  span where the transformed points are written
 * @param src  span where the points are read from
 *
 * Note: min(dst.size(), src.size()) is the number of points that will be written to dst.
 */
void mapHomogeneousPoints(SkSpan<SkPoint3> dst, SkSpan<const SkPoint3> src) const;
```
