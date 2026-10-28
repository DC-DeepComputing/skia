---
name: skia/bench/document-embedded-test-asset-provenance
description: When embedding static asset data, raw path strings, or test datasets in tests or benchmarks, document their source, originating asset, or provenance in a comment
rubric: readability
severity: nit
---

**Rule.** When embedding static asset data, raw path strings, or test datasets
in source code, add a comment documenting their source or provenance (such as
the originating SVG, SKP, or specification). Note the motivating workload, bug
reference, or specific pipeline path exercised by the asset.

**Why.** Without documenting the origin or provenance of embedded asset data,
reviewers and maintainers cannot verify licensing, understand whether the data
represents a realistic test case, or identify whether duplicate assets already
exist elsewhere in the codebase.

## Examples

Bad:

```cpp
static constexpr const char* kTigerPathStrings[] = {
    "M27.014 90.57s.039.744-.287.736...",
    // ...
};
```

Good:

```cpp
// Paths extracted from desk_tigersvg.skp that exercise the "inner triangulation"
// path through the tessellator (see b/123456789).
static constexpr const char* kTigerPathStrings[] = {
    "M27.014 90.57s.039.744-.287.736...",
    // ...
};
```
