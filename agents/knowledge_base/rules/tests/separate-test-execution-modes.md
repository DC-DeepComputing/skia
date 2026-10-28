---
name: skia/tests/separate-test-execution-modes
description: When unit tests compare multiple execution paths on shared inputs, define fixtures upfront and run each mode in separate loops rather than interleaving them
rubric: design
severity: nit
---

**Rule.** When authoring unit tests that evaluate or compare multiple execution
paths or implementations (such as CPU vs. GPU, direct math vs.
shader/color-filter, or distinct API pipelines) across a test suite, define
shared input datasets and expected fixtures upfront. Execute each implementation
mode in a dedicated loop or separate named helper function rather than
interleaving different execution modes inside a single compound loop.

**Why.** Interleaving setup, execution, and assertions for multiple execution
modes inside a single loop complicates loop state, obscures failure contexts,
and tangles intermediate allocations (such as bitmaps or surfaces). Separating
modes into distinct sequential loops or helpers keeps test preconditions
explicit and isolates failure reports.

## Examples

Bad:

```cpp
    for (size_t t = 0; t < kNumTests; ++t) {
        const auto headroom = testHeadrooms[t];
        // Test direct CPU evaluation.
        {
            SkColor4f colors[kNumColors];
            agtm.applyGain(SkSpan(colors, kNumColors), headroom);
            REPORTER_ASSERT(r, check_colors(colors, expected[t]));
        }
        // Interleaved test of color filter shader implementation.
        {
            SkBitmap bm;
            bm.allocPixels(...);
            canvas->drawImage(inputImage.get(), 0, 0, SkSamplingOptions(), &paint);
            REPORTER_ASSERT(r, check_bitmap(bm, expected[t]));
        }
    }
```

Good:

```cpp
    // Hoist test fixtures and expected values upfront.
    constexpr size_t kNumColors = 6;
    SkColor4f inputColors[kNumColors] = { ... };
    const float testHeadrooms[kNumTests] = { ... };
    SkColor4f expectedColors[kNumTests][kNumColors] = { ... };

    // Mode 1: evaluate direct CPU function in its own loop.
    for (size_t t = 0; t < kNumTests; ++t) {
        skiatest::ReporterContext ctx(r, SkStringPrintf("CPU headroom:%f", testHeadrooms[t]));
        agtm.applyGain(SkSpan(outputColors, kNumColors), testHeadrooms[t]);
        REPORTER_ASSERT(r, check_colors(outputColors, expectedColors[t]));
    }

    // Mode 2: evaluate color filter pipeline in a dedicated loop.
    for (size_t t = 0; t < kNumTests; ++t) {
        skiatest::ReporterContext ctx(r, SkStringPrintf("Filter headroom:%f", testHeadrooms[t]));
        canvas->drawImage(inputImage.get(), 0, 0, SkSamplingOptions(), &paint);
        REPORTER_ASSERT(r, check_bitmap(bm, expectedColors[t]));
    }
```
