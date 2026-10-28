---
name: skia/gpu/document-atlas-buffer-view-padding-semantics
description: When defining atlas padding APIs or backing zero-initialized allocations like sk_calloc_throw, document padding exclusion and that zero-initialization is load-bearing
rubric: readability
severity: should_fix
---

**Rule.** When declaring or modifying texture atlas or buffer-view APIs that
accept inset or padding parameters (such as prepForRender), document whether
returned views (e.g., SkPixmap) include or exclude the padding, and state
whether the padding region is guaranteed to be zero-initialized. Additionally,
at backing allocation sites (such as sk_calloc_throw), document that
zero-initialization is load-bearing so maintainers do not replace it with
uninitialized allocation (like malloc) for perceived performance.

**Why.** Callers rendering into atlas entries need to know whether returned
buffer bounds include border padding or only the writable inner bounds to
prevent texture filtering bleed. Furthermore, documenting load-bearing
zero-initialization at the allocation site ensures maintainers understand that
zeroing unwritten margins is an intentional API contract rather than unnecessary
overhead.

## Examples

Bad:

```cpp
// 1. Atlas buffer-view declaration:
SkPixmap prepForRender(const AtlasLocator&, int padding = 0);

// 2. Backing allocation:
if (!fData) {
    fData = reinterpret_cast<std::byte*>(sk_calloc_throw(this->rowBytes() * fHeight));
}
```

Good:

```cpp
// 1. Atlas buffer-view declaration documents padding exclusion & zeroing:
// Returns a Pixmap pointing to backing data. Pixmap excludes padding. The entire
// Plot is cleared to zero when allocated.
SkPixmap prepForRender(const AtlasLocator&, int padding = 0);

// 2. Backing allocation documents load-bearing zero-initialization contract:
if (!fData) {
    // We use calloc here because our contract is that all pixel data is initially zero.
    // This is of particular importance when a caller uses padding with prepForRender().
    fData = reinterpret_cast<std::byte*>(sk_calloc_throw(this->rowBytes() * fHeight));
}
```
