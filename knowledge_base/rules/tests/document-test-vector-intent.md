---
name: skia/tests/document-test-vector-intent
description: When defining test vectors, bespoke geometries, comparison tolerances, or coordinate pixel probes in tests, document regimes, geometry rationale, precision or error limits, and visual intent
rubric: readability
severity: nit
---

**Rule.** When defining arrays of hardcoded test vectors covering multiple
operational regimes (such as SDR vs. HDR or boundary coordinates), or
constructing bespoke test geometries and contours in GMs, document the
categories, coordinate offsets, and edge-case rationale being exercised. When
using comparison tolerances, epsilons, or non-zero error limits in numerical
tests (or tracking minor discrepancies without reporting failures), add comments
explaining how the threshold is derived or why noise is expected (e.g., due to
quantization or precision bounds). Similarly, when probing raster pixel outputs
at specific coordinates, explain the expected visual outcome and what the
assertion sequence verifies.

**Why.** Hardcoded numeric coordinates, pixel colors, test vectors, bespoke path
contours, and arbitrary comparison tolerances or error limits appear arbitrary
without contextual documentation. Documenting the specific operational regimes,
geometric edge cases (such as subpixel offsets or open vs. closed contours),
precision derivations justifying comparison epsilons, and expected noise floors
ensures test coverage is auditable during code review and prevents future
maintainers from inadvertently deleting necessary probes or regressing visual
coverage during refactoring.

## Examples

Bad:

```cpp
// Variant 1: Undocumented test vectors and arbitrary comparison epsilon or error limits
constexpr size_t kNumTestColors = 3;
SkColor4f inputTestColors[kNumTestColors] = {
    {1.00f, 1.00f, 1.00f, 1.f}, {1.00f, 0.50f, 0.25f, 1.f}, {4.00f, 4.00f, 4.00f, 1.f},
};
constexpr float kEpsilon = 0.01f;
constexpr uint32_t kErrorLimit = 3;
std::array<uint32_t, kErrorLimit> minorErrorCount = {0, 0, 0};

// Variant 2: Undocumented bespoke path contours and offsets in a GM
auto lineOnOpen = SkPathBuilder().lineTo(0, 5).lineTo(5, 5).detach();
auto lineOffClosed = SkPathBuilder(lineOnOpen).close().detach().makeOffset(0.5f, 0.5f);

// Variant 3: Undocumented coordinate pixel assertions
assert_exact_color("left", {3, 7}, SK_ColorWHITE);
assert_color_in_range("on line", {4, 7}, SK_ColorBLACK, SK_ColorGRAY);
```

Good:

```cpp
// Variant 1: Documented test vector regimes and derived epsilon / error tolerances
// Input colors in gain application space exercising SDR (<= 1.0) and HDR (> 1.0).
constexpr size_t kNumTestColors = 3;
SkColor4f inputTestColors[kNumTestColors] = {
    {1.00f, 1.00f, 1.00f, 1.f}, {1.00f, 0.50f, 0.25f, 1.f}, {4.00f, 4.00f, 4.00f, 1.f},
};
// Math uses at least half-precision; near 1.0 we maintain at least 10-bit precision.
constexpr float kEpsilon = 1.f / 1024.f;
// Track 1-3 sample errors without failing; some noise is expected due to LUT quantization.
constexpr uint32_t kErrorLimit = 3;
std::array<uint32_t, kErrorLimit> minorErrorCount = {0, 0, 0};

// Variant 2: Documented GM contours testing pixel-aligned vs. 0.5px subpixel offsets
// Row 1: on-pixel line, open contours (should not show caps).
auto lineOnOpen = SkPathBuilder().lineTo(0, 5).lineTo(5, 5).detach();
// Row 4: 0.5px off-pixel line, closed contours (verify cap extension behavior).
auto lineOffClosed = SkPathBuilder(lineOnOpen).close().detach().makeOffset(0.5f, 0.5f);

// Variant 3: Documented coordinate pixel assertions
// Verify the drawn contour is a square with 2px-wide gray lines and white fill.
assert_exact_color("left", {3, 7}, SK_ColorWHITE);
assert_color_in_range("on line", {4, 7}, SK_ColorBLACK, SK_ColorGRAY);
```
