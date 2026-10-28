---
name: skia/inline-hot-helpers-for-non-lto-builds
description: When defining functions for cross-translation-unit inlining, place definitions and mark them inline or static inline in headers, avoiding redundant inline in .cpp or unprofiled SK_ALWAYS_INLINE
rubric: safety
severity: should_fix
---

**Rule.** Define performance-sensitive, single-consumer, or small non-member
helper functions directly in headers (or in the consuming translation unit)
marked 'inline' or 'static inline' rather than in a separate .cpp file so
compilers can inline across callsites without LTO. Do not add redundant 'inline'
qualifiers to function definitions in .cpp files, and reserve 'SK_ALWAYS_INLINE'
for profiled hot paths rather than premature optimization.

**Why.** Skia builds developer tooling, tests, and benchmarks without Link-Time
Optimization (LTO), preventing cross-translation-unit inlining when function
definitions are isolated in .cpp files. Marking a definition 'inline' in a .cpp
file does not expose its body to other compilation units, while defining helper
functions in headers without 'inline' or 'static inline' risks duplicate static
function bodies or missed inlining. Forcing 'SK_ALWAYS_INLINE' prematurely
increases binary size without verified benefit.

## Examples

Bad:

```cpp
// DrawListTypes.cpp (isolated TU prevents cross-TU inlining without LTO)
BindingList* Layer::searchBinding(const LayerKey& key) {
    return findBinding(key);
}

// DrawAtlas.cpp (redundant 'inline' in .cpp does not allow cross-TU inlining)
inline std::optional<EntryID> DrawAtlas::Plot::makeEntry(SkISize size) {
    return fRectanizer.addRect(size.width(), size.height());
}

// SkPathOpsTSect.h (non-member helper in header not marked inline risks duplicate bodies)
static bool SkDoubleIsNaN(double x) {
    return x != x;
}
```

Good:

```cpp
// DrawListTypes.h (defined in header or consuming .cpp so compiler can inline)
struct Layer {
    BindingList* searchBinding(const LayerKey& key) { return findBinding(key); }
};

// DrawAtlas.h (defined in header for cross-TU callers; reserve SK_ALWAYS_INLINE for hot paths)
class Plot {
    std::optional<EntryID> makeEntry(SkISize size) {
        return fRectanizer.addRect(size.width(), size.height());
    }
};

// SkPathOpsTSect.h (explicitly marked static inline for cross-TU inlining)
static inline bool SkDoubleIsNaN(double x) {
    return x != x;
}
```
