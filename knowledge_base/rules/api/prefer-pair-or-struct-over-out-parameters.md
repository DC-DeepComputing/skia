---
name: skia/api/prefer-pair-or-struct-over-out-parameters
description: When designing functions producing results, transformations, or multiple outputs, return values, structs, pairs, or std::optional directly instead of mutating out-parameters
rubric: safety
severity: should_fix
---

**Rule.** When designing functions that produce results or transform small value
types, return the value directly—as a primitive, small value type, configured
struct, std::pair, or std::optional<T>—rather than mutating out-parameters
(Type*, Type**) or in-out pointers. Returning by value avoids forcing callers
with const sources to allocate temporary mutable buffers. For fallible
deserialization or parsing, return std::optional<T> by value with secondary
outputs via defaulted optional pointers (size_t* bytesRead = nullptr). Never
pass mutable out-parameters via non-const reference; pass unavoidable in-out
parameters as raw pointers.

**Why.** Returning values, configured structs, or fallible optional results
directly by value supports single-line initialization and clean caller control
flow, avoids temporary pre-declarations and address-of boilerplate, and makes
data flow explicit. For transformation helpers on small value types, returning
by value allows callers reading from const sources or read-only iterators to
avoid allocating temporary mutable buffers and copying data. Splitting outputs
between return values and out-parameters or using pointer-to-pointer/reference
wrappers obscures output state and complicates call sites.

## Examples

Bad:

```cpp
// Bad: mutating in-out pointer forces caller to allocate mutable temp buffer
static void force_small_to_zero(SkPoint* pt);
SkPoint pts[4];
memcpy(pts, pathPts, count * sizeof(SkPoint));
force_small_to_zero(&pts[0]);

// Bad: using output pointer parameters for single values or small value types
virtual void onGetFontStyle(SkFontStyle* fontStyle) const;

// Bad: taking an out-parameter for deserialized object and returning size/status
static size_t readFromMemory(SkPath* path, const void* buffer, size_t length);

// Bad: splits status flag and output object across return type and out-parameter
virtual bool asImageFilter(const SkMatrix& ctm, const SkPaint& paint,
                           sk_sp<SkImageFilter>& dstImageFilter) const;
```

Good:

```cpp
// Good: returning transformed value by value eliminates temporary copies
static SkPoint force_small_to_zero(const SkPoint& pt);
curve[0] = force_small_to_zero(pathPts[0]);

// Good: returns single value or small value type directly by value
virtual SkFontStyle onGetFontStyle() const;

// Good: returns std::optional<T> by value with optional secondary out-parameter
static std::optional<SkPath> ReadFromMemory(const void* buffer, size_t length,
                                            size_t* bytesRead = nullptr);

// Good: bundles outputs into std::pair or dedicated struct
virtual std::pair<sk_sp<SkImageFilter>, bool> asImageFilter(const SkMatrix& ctm,
                                                            const SkPaint& paint) const;
```
