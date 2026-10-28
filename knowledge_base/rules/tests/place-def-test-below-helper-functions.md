---
name: skia/tests/place-def-test-below-helper-functions
description: When authoring unit test files with dedicated helper functions, place DEF_TEST definitions immediately below the helper functions they invoke rather than grouping all helpers separately
rubric: readability
severity: should_fix
---

**Rule.** When authoring unit test files that use local static helper functions
to execute parameterized test scenarios, place the DEF_TEST (or DEF_GANESH_TEST
/ DEF_GRAPHITE_TEST) definitions immediately below the specific helper function
they invoke. Do not cluster all helper functions together at the top of the file
followed by all test definitions at the bottom, unless a helper is shared across
multiple unrelated test suites.

**Why.** Co-locating each helper function with the test cases that call it keeps
test setup and test invocation cohesive and modular. Readers can immediately see
how each test case configures and exercises the helper without jumping back and
forth across large test files.

## Examples

Bad:

```cpp
static void run_no_offset_test(skiatest::Reporter* reporter, SkPaint::Cap cap) {
    // helper logic ...
}

static void run_half_offset_test(skiatest::Reporter* reporter, SkPaint::Cap cap) {
    // helper logic ...
}

DEF_TEST(HairlineNoOffset_ButtCap, r) { run_no_offset_test(r, SkPaint::kButt_Cap); }
DEF_TEST(HairlineNoOffset_RoundCap, r) { run_no_offset_test(r, SkPaint::kRound_Cap); }
DEF_TEST(HairlineOffset_ButtCap, r) { run_half_offset_test(r, SkPaint::kButt_Cap); }
DEF_TEST(HairlineOffset_RoundCap, r) { run_half_offset_test(r, SkPaint::kRound_Cap); }
```

Good:

```cpp
static void run_no_offset_test(skiatest::Reporter* reporter, SkPaint::Cap cap) {
    // helper logic ...
}
DEF_TEST(HairlineNoOffset_ButtCap, r) { run_no_offset_test(r, SkPaint::kButt_Cap); }
DEF_TEST(HairlineNoOffset_RoundCap, r) { run_no_offset_test(r, SkPaint::kRound_Cap); }

static void run_half_offset_test(skiatest::Reporter* reporter, SkPaint::Cap cap) {
    // helper logic ...
}
DEF_TEST(HairlineOffset_ButtCap, r) { run_half_offset_test(r, SkPaint::kButt_Cap); }
DEF_TEST(HairlineOffset_RoundCap, r) { run_half_offset_test(r, SkPaint::kRound_Cap); }
```
