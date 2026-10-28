---
name: skia/core/ensure-non-null-typeface-with-empty-fallback
description: When initializing or updating font containers like SkFont, enforce a non-null typeface invariant using SkTypeface::MakeEmpty() instead of permitting nullptr
rubric: design
severity: should_fix
---

**Rule.** When constructing or updating font holders or wrappers (such as
SkFont), enforce the invariant that the stored typeface is never null by
substituting an empty typeface (SkTypeface::MakeEmpty()) when null or
unspecified. Do not store nullptr or require downstream callers to perform
defensive null checks, query default fallbacks, or assert non-nullness before
dereferencing.

**Why.** Allowing font wrappers to hold a null typeface leaks nullable state to
callers, requiring scattered defensive checks or risking null pointer
dereferences and assertion failures. Enforcing non-null typeface storage via the
Null Object pattern guarantees that methods like getTypeface() are
unconditionally safe to dereference.

## Examples

Bad:

```cpp
// Storing nullptr requires callers to defensively check or assert:
SkFont::SkFont(sk_sp<SkTypeface> face) : fTypeface(std::move(face)) {}

// Caller:
SkTypeface* typeface = font.getTypeface();
SkASSERT(typeface);
```

Good:

```cpp
// Enforce non-null invariant at construction via empty typeface fallback:
SkFont::SkFont(sk_sp<SkTypeface> face) : fTypeface(std::move(face)) {
    if (!fTypeface) {
        fTypeface = SkTypeface::MakeEmpty();
    }
}

// Caller:
SkTypeface* typeface = font.getTypeface(); // Unconditionally safe to dereference
```
