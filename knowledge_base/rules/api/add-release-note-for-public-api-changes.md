---
name: skia/api/add-release-note-for-public-api-changes
description: When modifying, deprecating, or removing public APIs, changing configuration contracts, or altering extensible virtual methods in public headers, add a release note in relnotes/
rubric: readability
severity: nit
---

**Rule.** When modifying, deprecating, or removing public APIs, altering virtual
methods on extensible base classes, or changing configuration contracts (such as
making optional struct members or parameters required) in public headers
(`include/**`), add a release note markdown file under `relnotes/` describing
the API change, deprecation, or removal along with migration guidance. Follow
release note formatting guidelines: do not use headings, do not start entries
with bullet points or asterisks, and do not reference local files.

**Why.** External clients and downstream projects consume public headers
directly and implement extensible Skia classes (such as `SkTypeface`). Breaking
signature changes, API deprecations or removals, and newly mandatory
configuration fields break downstream builds and runtime initialization without
advance notice; adding an individual note in `relnotes/` ensures release
automation aggregates migration guidance into `RELEASE_NOTES.md`.

## Examples

Bad:

```cpp
// include/core/SkTypeface.h
// Modifying a public virtual method signature without adding a release note in relnotes/:
virtual void getGlyphToUnicodeMap(SkSpan<SkUnichar> dstArray) const = 0;

// include/gpu/vk/VulkanBackendContext.h
// Making a previously optional struct member mandatory without a release note:
- // Optional. The client may provide an implementation of a VulkanMemoryAllocator for Skia to use
+ // The client must provide an implementation of a VulkanMemoryAllocator for Skia to use
  sk_sp<VulkanMemoryAllocator> fMemoryAllocator;

// include/core/SkPath.h
// Deprecating or removing public API SkPath::isArc without adding a release note in relnotes/:
- bool isArc(SkArc*) const;
```

Good:

```cpp
<!-- relnotes/typeface_virtuals_span.md -->
Virtuals in `SkTypeface` subclasses now take SkSpan instead of ptr/count. This is part of the larger change where public APIs are being converted to take SkSpan where applicable.

<!-- relnotes/vma.md -->
`VulkanBackendContext::fMemoryAllocator` is no longer optional.

<!-- relnotes/asArc.md -->
`SkPath::isArc()` has been removed. This method reported true if the path was internally recognized as an "Arc" segment. This functionality is now removed, so the method has also been removed.
```
