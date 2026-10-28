---
name: skia/tests/consistent-test-name-prefix
description: When naming unit tests (DEF_TEST), adhere to established prefixes for dm --match and selective CI bot filters, and structure names as Operation_InputState_ExpectedOutput
rubric: testing
severity: should_fix
---

**Rule.** When authoring or adding unit tests (DEF_TEST, DEF_SERIAL_TEST,
DEF_GRAPHITE_TEST, or DEF_GANESH_TEST), adhere strictly to the established test
name prefix used across that test file for filtering via dm --match; for
features run under selective CI configurations, ensure test names share an
existing matched substring or update the bot's --match filter in
infra/bots/gen_tasks_logic/dm_flags.go. Additionally, structure multi-part test
names to explicitly indicate operation, input state or conditions, and expected
output (Operation_InputState_ExpectedOutput), placing modifiers characterizing
input geometry or state in the input condition segment rather than in the
expected output segment.

**Why.** Inconsistent test prefixes disrupt developer filtering workflows via dm
--match and cause tests to be silently skipped on CI bots that execute selective
test subsets via --match filters. Furthermore, vague test names or misplacing
input geometry conditions into the expected output segment obscures test intent
and failure context by conflating what is being tested with what is being
asserted.

## Examples

Bad:

```cpp
// In tests/SkPngRustEncoderTest.cpp (fails to match bot filter 'RustPngCodec' in dm_flags.go)
DEF_TEST(Encode_png_Rust_smoke_test, r) {
    // ...
}

// In tests/SkBmpRustDecoderTest.cpp (violates file's established prefix convention)
DEF_TEST(Codec_Bmp_Rust_IncrementalDecode, r) {
    // ...
}

// In tests/CappedHairlinesTest.cpp (input condition 'OpenContour' misplaced in output segment)
DEF_TEST(DrawAntialiasHairline_SquareCappedHalfPixelOffset_SinglePixelWideOpenContourOutput,
         reporter) {
    // ...
}
```

Good:

```cpp
// In tests/SkPngRustEncoderTest.cpp (shares prefix and added to --match in dm_flags.go)
DEF_TEST(RustEncodePng_smoke_test, r) {
    // ...
}

// In tests/SkBmpRustDecoderTest.cpp (consistent prefix for 'dm --match RustBmpCodec')
DEF_TEST(RustBmpCodec_IncrementalDecode, r) {
    // ...
}

// In tests/CappedHairlinesTest.cpp (structured: Operation_InputState_ExpectedOutput)
DEF_TEST(DrawAntialiasHairline_SquareCappedHalfPixelOffsetOpenContour_SinglePixelWideOutput,
         reporter) {
    // ...
}
```
