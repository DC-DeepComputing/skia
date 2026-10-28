---
name: skia/codec/codec-test-experimental-subsystems-with-dedicated-unit-tests
description: When implementing new features or codec capabilities in experimental subsystems unable to run general test suites, add dedicated unit tests covering key modes
rubric: testing
severity: should_fix
---

**Rule.** When adding features or format capabilities (such as subset or
incremental decoding) to experimental or evolving codecs and subsystems that
cannot yet execute the general test suite (such as CodecTest.cpp), add dedicated
unit tests under tests/ covering key modes and edge cases rather than deferring
test coverage to future test suite integration.

**Why.** Deferring test coverage until an evolving subsystem is integrated into
broad test suites leaves new code paths unvalidated in continuous integration.
Dedicated unit tests catch regressions immediately and ensure operational
variations (such as progressive passes, subsets, or streaming chunks) are
verified against baseline expectations.

## Examples

Bad:

```cpp
// Adding subset decoding to an experimental codec without unit test coverage,
// planning to rely on broad test suite integration (e.g. CodecTest.cpp) in a future CL.
bool SkPngRustCodec::onGetSubset(const SkIRect* desiredSubset) {
    ...
}
```

Good:

```cpp
static void test_subset_decode(skiatest::Reporter* r, const char* resource) {
    ...
    SkIRect subset = SkIRect::MakeLTRB(left, top, right, bottom);
    SkCodec::Options options;
    options.fSubset = &subset;
    REPORTER_ASSERT(r, SkCodec::kSuccess == codec->startIncrementalDecode(
        info, tiledBM.getAddr(left, top), tiledBM.rowBytes(), &options));
    REPORTER_ASSERT(r, SkCodec::kSuccess == codec->incrementalDecode());
}

DEF_TEST(RustPngCodec_subset, r) {
    test_subset_decode(r, "images/baby_tux.png");
    test_subset_decode(r, "images/plane_interlaced.png");
}
```
