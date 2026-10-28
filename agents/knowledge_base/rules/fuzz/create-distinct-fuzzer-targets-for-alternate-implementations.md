---
name: skia/fuzz/create-distinct-fuzzer-targets-for-alternate-implementations
description: When adding fuzzing coverage for alternate implementations or backends in fuzz/, define a distinct fuzzer target instead of conditionally replacing existing fuzzers with preprocessor guards
rubric: testing
severity: should_fix
---

**Rule.** When adding fuzzing coverage for a new or alternate implementation
(such as a Rust-based codec or alternative backend), define a separate, distinct
fuzzer target (e.g., via DEF_FUZZ or a dedicated LLVMFuzzerTestOneInput binary)
rather than conditionally branching with #if / #else inside an existing fuzzer.
Guard each distinct fuzzer definition with its corresponding build macro (e.g.,
#if defined(SK_CODEC_ENCODES_PNG_WITH_LIBPNG) vs. #if
defined(SK_CODEC_ENCODES_PNG_WITH_RUST)).

**Why.** In continuous fuzzing harnesses like OSS-Fuzz and ClusterFuzz, each
fuzzer target corresponds to an independent executable, crash triage bucket, and
seed corpus. Conditionally swapping implementations inside an existing target
hides crashes behind build flags, impairs corpus evolution, and prevents running
continuous fuzz coverage for both implementations simultaneously.

## Examples

Bad:

```cpp
DEF_FUZZ(PNGEncoder, fuzz) {
    auto bm = make_fuzzed_bitmap(fuzz);
#if defined(SK_CODEC_ENCODES_PNG_WITH_RUST)
    auto opts = SkPngRustEncoder::Options{};
    std::ignore = SkPngRustEncoder::Encode(bm.pixmap(), opts);
#else
    auto opts = SkPngEncoder::Options{};
    std::ignore = SkPngEncoder::Encode(bm.pixmap(), opts);
#endif
}
```

Good:

```cpp
#if defined(SK_CODEC_ENCODES_PNG_WITH_LIBPNG)
DEF_FUZZ(PNGEncoder, fuzz) {
    auto bm = make_fuzzed_bitmap(fuzz);
    auto opts = SkPngEncoder::Options{};
    std::ignore = SkPngEncoder::Encode(bm.pixmap(), opts);
}
#endif

#if defined(SK_CODEC_ENCODES_PNG_WITH_RUST)
DEF_FUZZ(PNGRustEncoder, fuzz) {
    auto bm = make_fuzzed_bitmap(fuzz);
    auto opts = SkPngRustEncoder::Options{};
    std::ignore = SkPngRustEncoder::Encode(bm.pixmap(), opts);
}
#endif
```
