---
name: skia/api/update-doc-comments-for-optional-returns
description: When documenting non-void functions, explicitly describe return value semantics (or simplify unused returns to void) and verify described values match the actual return type
rubric: readability
severity: should_fix
---

**Rule.** In doc comments for non-void functions, explicitly describe return
value semantics (or simplify the signature to void if callers ignore the
result). Additionally, verify described fallback and sentinel values match the
actual return type: describe boolean failure as false rather than nullptr;
describe std::optional<T> fallbacks as {} or std::nullopt rather than false or
0; describe pointer wrappers (such as BufferWriter) using their wrapper type and
invalid state rather than raw or null pointers; and describe unsigned integer
sentinels (e.g., uint16_t or size_t) as zero rather than negative values.

**Why.** Doc comments that omit return value descriptions leave callers guessing
what a non-void return signifies and hide opportunities to simplify unused
returns to void. Describing return values that contradict the function signature
(such as stating nullptr for bool, false for std::optional, raw pointers for
BufferWriter, or negative sentinels for unsigned integers) confuses callers
about return semantics and error handling.

## Examples

Bad:

```cpp
// Variant 1: Bool function documented as returning nullptr
/**
 * Populate out parameters. If tone mapping should not be performed, return nullptr.
 */
bool PopulateToneMapParams(const Metadata& metadata, ToneMapParams* outParams);

// Variant 2: std::optional function with stale bool return comment
/**
 * Returns oval info if path is an oval, else return false.
 */
std::optional<SkPathOvalInfo> isOval() const;

// Variant 3: Pointer wrapper documented as raw or null pointer
/**
 * Return a pointer to mapped range. A null pointer is returned on failure.
 */
std::pair<BufferWriter, BindBufferInfo> getMappedSubrange(size_t count, size_t stride);

// Variant 4: Unsigned integer documented with negative sentinel
/**
 * Gets the key size. A negative value is returned if the shape has no key.
 */
uint16_t unstyledKeySize() const;

// Variant 5: Non-void function doc comment omitting return value description
/* Extend points in the direction of the tangent by 1/2 unit.
   If the curve is degenerate, move the cap out 1/2 unit horizontally. */
template <SkPaint::Cap capStyle>
bool extend_pts(std::optional<SkPathVerb> prev, SkSpan<SkPoint> pts);
```

Good:

```cpp
// Variant 1: Bool function documented as returning false
/**
 * Populate out parameters. If tone mapping should not be performed, return false.
 */
bool PopulateToneMapParams(const Metadata& metadata, ToneMapParams* outParams);

// Variant 2: std::optional function documented with empty fallback
/**
 * Returns oval info if path is an oval, else return {}.
 */
std::optional<SkPathOvalInfo> isOval() const;

// Variant 3: Pointer wrapper and invalid state accurately documented
/**
 * Return a mapped range wrapped in BufferWriter. An invalid BufferWriter is returned on failure.
 */
std::pair<BufferWriter, BindBufferInfo> getMappedSubrange(size_t count, size_t stride);

// Variant 4: Unsigned integer documented with zero sentinel
/**
 * Gets the key size. A zero value is returned if the shape has no key.
 */
uint16_t unstyledKeySize() const;

// Variant 5: Return value explicitly documented (or simplified to void)
/* Extend points in the direction of the tangent by 1/2 unit.
   Returns true if the curve was degenerate. */
template <SkPaint::Cap capStyle>
bool extend_pts(std::optional<SkPathVerb> prev, SkSpan<SkPoint> pts);

// Or simplify to void if callers do not need the return value:
template <SkPaint::Cap capStyle>
void extend_pts(std::optional<SkPathVerb> prev, SkSpan<SkPoint> pts);
```
