---
name: skia/pass-nullopt-for-omitted-optional-argument
description: When omitting an optional value in a function call expecting std::optional<T>, pass std::nullopt instead of a default-constructed object T()
rubric: correctness
severity: should_fix
---

**Rule.** When calling a function with a parameter expecting std::optional<T>,
pass std::nullopt rather than a default-constructed object T() to indicate the
absence of a value. Do not pass an empty or default-constructed instance to an
optional parameter unless the callee explicitly requires an engaged optional
containing a default value.

**Why.** Passing a default-constructed object T() implicitly constructs an
engaged std::optional<T> containing the default instance. When the callee checks
if (opt) or opt.has_value() to determine whether a value was supplied or needs
to be computed or generated, the engaged optional evaluates to true,
unintentionally bypassing fallback or generation logic and operating on an
uninitialized or empty object.

## Examples

Bad:

```cpp
void SkScalerContext::getPath(SkGlyph& glyph, SkArenaAlloc* alloc) {
    this->internalGetPath(glyph, alloc, GeneratedPath());
}
```

Good:

```cpp
void SkScalerContext::getPath(SkGlyph& glyph, SkArenaAlloc* alloc) {
    this->internalGetPath(glyph, alloc, std::nullopt);
}
```
