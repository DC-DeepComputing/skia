---
name: skia/api/avoid-redundant-convenience-methods
description: When designing class interfaces or helper APIs, avoid redundant convenience methods, duplicate accessor aliases, fallback wrappers, single-caller forwarders, or inverted query aliases
rubric: design
severity: should_fix
---

**Rule.** When designing class interfaces, container APIs, or helper functions,
avoid adding redundant convenience wrappers, duplicate accessor aliases,
default-fallback methods, single-caller forwarders, or trivial inverted aliases
when canonical operations suffice. Do not introduce duplicate accessors (such as
size() alongside count()), boolean convenience wrappers over richer queries, or
inverted wrappers (such as IsDstReadRequired returning !CanUseHardwareBlending);
expose the primary query and let callers negate or convert directly. For
fallible operations, return std::optional<T> rather than fallback objects,
letting callers use .value_or(). Call external APIs directly instead of adding
single-caller forwarding wrappers (like wrapping installPixels).

**Why.** Thin convenience wrappers, duplicate accessor aliases, trivial inverted
queries, and single-caller forwarding methods bloat API surfaces, introduce
unnecessary indirection, and create maintenance overhead without meaningful
abstraction. Furthermore, returning default-constructed fallback objects upon
failure destroys information by conflating genuinely empty results with failures
or unrepresentable items.

## Examples

Bad:

```cpp
// Redundant alias accessor on container:
class DenseBiMap {
public:
    int count() const { return fIndexToData.size(); }
    size_t size() const { return fIndexToData.size(); }
};

// Redundant boolean wrapper on builder:
bool computeIsConvex() const { return SkPathConvexity_IsConvex(this->computeConvexity()); }

// Trivial inverted alias function:
bool IsDstReadRequired(const Caps* caps, std::optional<SkBlendMode> blendMode, Coverage cov) {
    return !CanUseHardwareBlending(caps, blendMode, cov);
}

// Information-destroying fallback convenience method:
SkPath getPath(SkGlyphID id) const {
    auto path = this->getIfPath(id);
    return path ? *path : SkPath();
}

// Single-caller forwarding wrapper method:
bool installIntoBitmap(SkBitmap* bm) {
    return bm->installPixels(SkImageInfo::MakeA8(fBounds.width(), fBounds.height()), image(), fRowBytes);
}
```

Good:

```cpp
// Retain canonical accessor; call sites use existing method:
class DenseBiMap {
public:
    int count() const { return fIndexToData.size(); }
};
drawPass->fPipelineDrawAreas.push_back_n(pipelineCache.count(), 0.f);

// Retain canonical query; call site converts with utility helper:
bool isConvex = SkPathConvexity_IsConvex(builder.computeConvexity());

// Expose primary function; call sites negate directly:
bool dstReadRequired = !CanUseHardwareBlending(caps, blendMode, cov);

// Return std::optional<T>; call site opts into fallback via value_or():
SkPath path = font.getPath(glyphID).value_or(SkPath());

// Call underlying general API directly at the call site:
bitmap.installPixels(SkImageInfo::MakeA8(dst->fBounds.width(), dst->fBounds.height()),
                     dst->image(), dst->fRowBytes);
```
