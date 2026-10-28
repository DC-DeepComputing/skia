---
name: skia/dont-return-false-for-optional
description: When signaling failure in functions returning std::optional, return {} or std::nullopt instead of return false or boolean error macros like HRB/HRBM
rubric: correctness
severity: blocking
---

**Rule.** When signaling failure from a function returning std::optional<T>,
return {} or std::nullopt rather than return false; or error-bailout macros that
return false or 0. In particular, when T is an arithmetic, scalar, enum, or
boolean type (such as SkScalar or int), returning false silently constructs an
engaged optional holding zero. When using platform error macros (such as HRB or
HRBM in Windows code), do not use boolean bailout variants in functions
returning std::optional; use HR_GENERAL(ex, msg, {}) or an appropriate
optional-returning error handler instead.

**Why.** In C++, bool implicitly converts to arithmetic and scalar types (false
converts to 0 or 0.0f), so returning false accidentally constructs an engaged
optional holding zero rather than an empty optional, causing callers to proceed
as if a valid value was returned. Furthermore, boolean error-bailout macros like
HRB and HRBM expand to return false, which causes compilation errors or
constructs an engaged optional instead of returning an empty optional ({}) on
failure.

## Examples

Bad:

```cpp
// Bug 1: Implicit conversion constructs engaged optional holding 0.0f
std::optional<SkScalar> getContourHeight(SkGlyphID glyph) {
    if (!glyph) {
        return false;
    }
    return computeHeight(glyph);
}

// Bug 2: HRBM expands to 'return false;', failing or constructing engaged optional
std::optional<SkPath> getPath() {
    HRBM(fFontFace->GetGlyphRunOutline(...), "Failed outline");
    return path;
}
```

Good:

```cpp
// Returns empty (disengaged) std::optional
std::optional<SkScalar> getContourHeight(SkGlyphID glyph) {
    if (!glyph) {
        return {};
    }
    return computeHeight(glyph);
}

// Uses HR_GENERAL with {} to return empty optional on failure
std::optional<SkPath> getPath() {
    HR_GENERAL(fFontFace->GetGlyphRunOutline(...), "Failed outline", {});
    return path;
}
```
