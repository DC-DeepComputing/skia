---
name: skia/tests/assert-intended-behavior-over-buggy-output
description: In unit and regression tests, assert expected correct behavior rather than codifying buggy output, and update bailout assertions when fixes enable successful results
rubric: correctness
severity: should_fix
---

**Rule.** When authoring unit or regression tests for an unfixed bug or new
feature, assert the intended correct behavior (so the test fails before the fix
is implemented) rather than asserting current buggy output to pass CI.
Additionally, when an algorithmic fix enables path rendering, triangulation,
geometry, or decoding routines to successfully process inputs that previously
failed gracefully or bailed out (such as returning 0 vertices, false, or null),
update existing regression tests that asserted failure or bailout to verify the
valid generated output instead.

**Why.** Asserting current buggy output in tests codifies defects and creates a
false sense of test health. Similarly, leaving regression tests asserting
graceful failure or zero output after an algorithm fix prevents testing that
valid geometry or data is actually generated, masking future regressions where
operations might unexpectedly revert to bailing out.

## Examples

Bad:

```cpp
// Bad: asserting current incorrect output to pass CI before a bug fix lands
DEF_TEST(HairlineOffsetBug, reporter) {
    ...
    assert_exact_color("on west line", {4, 7}, SK_ColorGRAY);
}

// Bad: leaving a regression test asserting bailout after a fix enables valid triangulation
DEF_TEST(Triangulator_Bug, reporter) {
    ...
    int vertexCount = GrAATriangulator::PathToAATriangles(path, kTol, clipBounds, &alloc);
    REPORTER_ASSERT(reporter, vertexCount == 0);
}
```

Good:

```cpp
// Good: asserting expected correct behavior so the test verifies the fix once implemented
DEF_TEST(HairlineOffsetBug, reporter) {
    ...
    assert_color_in_range("on west line", {4, 7}, SK_ColorBLACK, SK_ColorGRAY);
}

// Good: updating the assertion to verify valid generated output after the fix
DEF_TEST(Triangulator_Bug, reporter) {
    ...
    int vertexCount = GrAATriangulator::PathToAATriangles(path, kTol, clipBounds, &alloc);
    REPORTER_ASSERT(reporter, vertexCount == 303);
}
```
