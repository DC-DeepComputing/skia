---
name: skia/avoid-implicit-span-from-unique-ptr-array
description: When passing dynamically allocated arrays in std::unique_ptr<T[]> to SkSpan parameters, explicitly construct SkSpan(ptr.get(), size) instead of relying on implicit conversion
rubric: correctness
severity: blocking
---

**Rule.** Do not pass dynamically allocated arrays managed by
std::unique_ptr<T[]> directly to functions or constructors expecting SkSpan<T>,
and do not rely on implicit container conversion in aggregate initialization.
Explicitly construct the span with SkSpan(ptr.get(), size) or use a container
whose size is tracked at runtime (such as std::vector or
skia_private::AutoSTArray).

**Why.** SkSpan's container-converting constructor requires std::data() and
std::size() to be well-formed. Because std::unique_ptr<T[]> does not track array
bounds at runtime, std::size() fails to resolve and implicit conversion fails to
compile, often causing build breaks on platform-specific code paths. Explicit
span construction or sized containers provide the necessary size metadata.

## Examples

Bad:

```cpp
std::unique_ptr<SkColor4f[]> colors(new SkColor4f[stops.size()]);
std::unique_ptr<SkScalar[]> pos(new SkScalar[stops.size()]);
// Compilation failure: std::size() is not defined for std::unique_ptr<T[]>
skPaint.setShader(SkShaders::TwoPointConicalGradient(
    start, startRadius, end, endRadius,
    {{colors, pos, tileMode}, interpolation}));
```

Good:

```cpp
// Option 1: Explicitly pass pointer and count to SkSpan
std::unique_ptr<SkColor4f[]> colors(new SkColor4f[stops.size()]);
std::unique_ptr<SkScalar[]> pos(new SkScalar[stops.size()]);
skPaint.setShader(SkShaders::TwoPointConicalGradient(
    start, startRadius, end, endRadius,
    SkGradient(
        SkGradient::Colors(SkSpan(colors.get(), stops.size()),
                           SkSpan(pos.get(), stops.size()),
                           tileMode),
        interpolation)));

// Option 2: Use sized containers that convert implicitly to SkSpan
std::vector<SkColor4f> colors(stops.size());
std::vector<SkScalar> pos(stops.size());
skPaint.setShader(SkShaders::TwoPointConicalGradient(
    start, startRadius, end, endRadius,
    SkGradient(SkGradient::Colors(colors, pos, tileMode), interpolation)));
```
