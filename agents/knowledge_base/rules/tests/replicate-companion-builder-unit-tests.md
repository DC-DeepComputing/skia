---
name: skia/tests/replicate-companion-builder-unit-tests
description: When adding or migrating methods to a companion or builder class, replicate or adapt corresponding unit tests from the original class to ensure behavioral parity
rubric: testing
severity: should_fix
---

**Rule.** When adding or migrating methods from an existing class to a companion
or builder class (such as mirroring SkPath operations into SkPathBuilder),
replicate or adapt the corresponding unit tests from the original class's test
suite to cover the new methods. Ensure that the replicated tests verify
behavioral parity, boundary conditions, and edge cases (such as empty objects,
single elements, and degenerate inputs) established by the original test cases.

**Why.** Companion and builder classes are designed to replace or complement
existing APIs while preserving expected semantics. Replicating established unit
tests guarantees behavioral parity and prevents subtle regressions across edge
cases that were already vetted and protected in the original class's test suite.

## Examples

Bad:

```cpp
// Added getLastPt() and transform() to SkPathBuilder, but only wrote a basic sanity test:
DEF_TEST(SkPathBuilder_basic, reporter) {
    SkPathBuilder b;
    b.moveTo(10, 10);
    REPORTER_ASSERT(reporter, b.getLastPt().has_value());
}
```

Good:

```cpp
// Replicate comprehensive test coverage from tests/PathTest.cpp into tests/PathBuilderTest.cpp:
DEF_TEST(SkPathBuilder_getLastPt, reporter) {
    SkPathBuilder b;
    REPORTER_ASSERT(reporter, b.getLastPt() == std::nullopt);
    b.moveTo(10, 10);
    REPORTER_ASSERT(reporter, b.getLastPt() == SkPoint::Make(10, 10));
    b.rLineTo(10, 10);
    REPORTER_ASSERT(reporter, b.getLastPt() == SkPoint::Make(20, 20));
}
```
