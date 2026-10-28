---
name: skia/tests/avoid-conditional-branching-in-tests
description: In tests and test loops, avoid conditional branching (if) to guard assertions or execution; assert exact states and test predicates deterministically
rubric: testing
severity: blocking
---

**Rule.** Do not use conditional branching (`if`) in unit tests or parameterized
test loops to execute assertions conditionally, tolerate alternate outcomes, or
guard downstream checks on deterministic inputs. Assert exact expected states
and return codes deterministically with `REPORTER_ASSERT` or framework
assertions (`expect`). When testing distinct variants or optional features, keep
execution linear by splitting scenarios into dedicated test functions or
modeling test data to run uniform assertions across all cases. Additionally, do
not test predicate or validation functions by placing them inside `if`
conditionals in rendering or integration tests; verify predicate functions in
dedicated unit tests with explicit assertions verifying both positive (true) and
negative (false) outputs, keeping rendering tests unconditional.

**Why.** In tests, deterministic inputs should yield predictable results.
Conditional branches (`if`) can silently conceal skipped assertions if an
earlier step fails unexpectedly, mask regressions by accepting divergent
behaviors, and leave negative outcomes unverified when predicate guards fail and
cause downstream null references or silent skips. Linear, deterministic tests
ensure failures are easy to trace and every expected assertion executes.

## Examples

Bad:

```cpp
// Anti-pattern 1: Branching on test case properties in parameterized loops
for (const auto& test : kTestCases) {
    skiatest::ReporterContext ctx(r, test.name);
    REPORTER_ASSERT(r, parse(test.primaryData));
    if (test.alternateData) {
        REPORTER_ASSERT(r, parse(test.alternateData));
    }
}

// Anti-pattern 2: Tolerating alternate outcomes or guarding assertions on fixed inputs
std::unique_ptr<SkCodec> codec = SkBmpRustDecoder::Decode(std::move(stream), &result);
if (codec) {
    SkCodec::Result decodeResult = codec->getPixels(info, pixels, rowBytes);
    REPORTER_ASSERT(r, decodeResult != SkCodec::kSuccess);
}

// Anti-pattern 3: Using predicate functions as conditional guards in rendering tests
if (CanvasKit.Path.CanInterpolate(path1, path2)) {
    const interp = CanvasKit.Path.MakeFromPathInterpolation(path1, path2, 0.5);
    canvas.drawPath(interp, paint);
}
```

Good:

```cpp
// Pattern 1: Structure test data so all iterations run uniform assertions, or split tests
for (const auto& test : kTestCases) {
    for (const auto& encoding : test.encodings) {
        skiatest::ReporterContext ctx(r, encoding.name);
        REPORTER_ASSERT(r, parse(encoding.data));
    }
}

// Pattern 2: Assert exact intermediate states and expected return codes deterministically
std::unique_ptr<SkCodec> codec = SkBmpRustDecoder::Decode(std::move(stream), &result);
REPORTER_ASSERT(r, codec && result == SkCodec::kSuccess);
if (!codec) {
    return;
}
SkCodec::Result decodeResult = codec->getPixels(info, pixels, rowBytes);
REPORTER_ASSERT(r, decodeResult == SkCodec::kIncompleteInput);

// Pattern 3: Explicitly assert true and false predicate outputs in dedicated unit tests
expect(CanvasKit.Path.CanInterpolate(path1, path2)).toBe(true);
expect(CanvasKit.Path.CanInterpolate(path1, path3)).toBe(false);
const interp = CanvasKit.Path.MakeFromPathInterpolation(path1, path2, 0.5);
canvas.drawPath(interp, paint);
```
