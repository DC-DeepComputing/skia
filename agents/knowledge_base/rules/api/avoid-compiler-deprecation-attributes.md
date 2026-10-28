---
name: skia/api/avoid-compiler-deprecation-attributes
description: When deprecating public APIs in headers, avoid [[deprecated]] attributes until callers migrate, and direct users to public or stdlib APIs in deprecation comments
rubric: readability
severity: blocking
---

**Rule.** Do not annotate legacy public APIs with [[deprecated]] while internal
callers or downstream clients still use them; migrate all internal call sites
upfront, and delay compiler deprecation attributes until downstream clients have
migrated. Deprecate public APIs using documentation comments (such as //
DEPRECATED) or build flag guards (such as SK_SUPPORT_LEGACY_*) instead. In
deprecation comments and notices, direct users to public Skia APIs or C++
standard library alternatives, never recommending internal or private APIs (such
as functions or types from include/private/ or src/).

**Why.** Skia and downstream embedders like Chromium and Android compile with
warnings treated as errors (-Werror or /WX); marking an API with [[deprecated]]
immediately breaks builds before callers migrate. Furthermore, headers in
include/private/ and src/ are excluded from client header search paths and
represent unstable implementation details, so recommending them in deprecation
notices encourages downstream clients to depend on unsupported private
interfaces.

## Examples

Bad:

```cpp
// In public header include/pathops/SkPathOps.h (using [[deprecated]] before callers migrate):
[[deprecated]]
static inline bool Simplify(const SkPath& path, SkPath* resultPath) {
    return Simplify(path, resultPath);
}

// In include/core/SkScalar.h (recommending private header API in deprecation notice):
// DEPRECATED: use SkIsNaN or std::isnan instead.
static inline bool SkScalarIsNaN(SkScalar x) { return SkIsNaN(x); }
```

Good:

```cpp
// In public header include/pathops/SkPathOps.h (comment-based deprecation with public alternative):
// DEPRECATED: Migrate callers to std::optional<SkPath> Simplify(const SkPath&).
static inline bool Simplify(const SkPath& path, SkPath* resultPath) {
    return Simplify(path, resultPath);
}

// In include/core/SkScalar.h (recommending standard library or public API alternative):
// DEPRECATED: use std::isnan instead.
static inline bool SkScalarIsNaN(SkScalar x) { return SkIsNaN(x); }
```
