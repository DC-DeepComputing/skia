---
name: skia/pass-wrapper-context-to-c-callbacks
description: When invoking C-style callback APIs accepting void* user data, pass the wrapper or sink object expected by the callback rather than the inner wrapped object
rubric: correctness
severity: blocking
---

**Rule.** When invoking C-style callback APIs that accept an untyped void*
user-data or context parameter (such as FreeType's FT_Outline_Decompose or
CoreGraphics' CGPathApply), pass the pointer to the wrapper, adapter, or sink
object expected by the callback functions rather than the underlying wrapped
object. Ensure the object passed matches the exact type that the callback
functions cast the context pointer to.

**Why.** Untyped void* context arguments bypass compiler type checking,
implicitly accepting any pointer. Passing the inner wrapped object (such as
SkPathBuilder*) instead of the wrapper or sink adapter compiles without warning,
but causes type confusion and memory corruption when callbacks cast the context
pointer back to the wrapper type.

## Examples

Bad:

```cpp
SkFTGeometrySink sink{builder};
if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE ||
    FT_Outline_Decompose(&face->glyph->outline, &SkFTGeometrySink::Funcs, builder)) {
    return false;
}
```

Good:

```cpp
SkFTGeometrySink sink{builder};
if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE ||
    FT_Outline_Decompose(&face->glyph->outline, &SkFTGeometrySink::Funcs, &sink)) {
    return false;
}
```
