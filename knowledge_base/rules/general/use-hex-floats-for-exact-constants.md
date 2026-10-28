---
name: skia/use-hex-floats-for-exact-constants
description: When declaring exact float constants, lookup tables, or fuzzer regression tests, use hexadecimal floats or dumpHex with SkBits2Float instead of decimal notation
rubric: testing
severity: should_fix
---

**Rule.** When generating, declaring, or formatting floating-point constants,
math lookup tables, or fuzzer regression tests where bit-exact precision is
critical, use hexadecimal floating-point notation (`%a` / `0x...p...f`) or
`SkBits2Float(0x...)` rather than rounded decimal or scientific notation. When
capturing reproducers for fuzzer bugs or path crashes, dump exact geometry using
`path.dumpHex()` (or `dump(nullptr, /*hex=*/true)`) and construct regression
unit tests with `SkBits2Float` literals to preserve exact bit patterns and
denormalized values. Include decimal approximations in comments if human
readability is needed.

**Why.** Decimal representations of floating-point values suffer from rounding
error and compiler parsing discrepancies across platforms. Hexadecimal float
literals and SkBits2Float format specifiers represent IEEE 754 bit patterns
exactly, eliminating rounding drift in sensitive mathematical matrices and
ensuring fuzzer reproduction unit tests trigger edge-case crashes reliably.

## Examples

Bad:

```cpp
// Bad: decimal scientific notation introduces roundtrip precision loss in tables
SkDebugf(" %1.9ef,", v);
constexpr float kRgbToYuvTable[] = { 2.989999950e-01f, 5.869999528e-01f, 1.140000001e-01f };

// Bad: capturing fuzzer regression test with rounded decimal float literals
DEF_TEST(PathFuzz_b123, r) {
    SkPath path = SkPathBuilder()
        .moveTo(3.18128e+11f, -1.30533e+11f)
        .detach();
    // Test may fail to reproduce crash due to decimal rounding of edge-case floats
}
```

Good:

```cpp
// Good: exact hexadecimal float format preserves bit-level IEEE 754 values
SkDebugf(" %16af,", v);
constexpr float kRgbToYuvTable[] = { 0x1.322d0ep-2f, 0x1.2c8b42p-1f, 0x1.d2f1aap-4f };

// Good: captured using path.dumpHex() with exact SkBits2Float literals
DEF_TEST(Triangulator_bug421959607, r) {
    SkPathBuilder path(SkPathFillType::kWinding);
    path.moveTo(SkBits2Float(0x529423dc), SkBits2Float(0xd1f322c7));
    int count = GrAATriangulator::PathToAATriangles(path.detach(), GrPathUtils::kDefaultTolerance, clipBounds, &alloc);
    REPORTER_ASSERT(r, count);
}
```
