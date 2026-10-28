---
name: skia/tests/update-or-replace-obsolete-tests
description: 'When changing API semantics or behavior, update or replace obsolete unit tests to verify the new behavior instead of disabling them with #if 0'
rubric: testing
severity: should_fix
---

**Rule.** When changing API semantics or component behavior, do not comment out
or disable failing unit tests with `#if 0` or delete them without replacement.
Update existing test assertions to verify the intended new semantics, or replace
obsolete tests with new test cases that explicitly validate the updated
behavior.

**Why.** Disabling or deleting failing tests removes test coverage and leaves
both legacy assumptions unverified and new behavior untested. Updating
assertions or replacing obsolete tests ensures that the test suite actively
validates the new semantics and protects against unintended regressions.

## Examples

Bad:

```cpp
// Bad: Disabling failing assertions with #if 0 when moveTo semantics change
#if 0
    // No matter how many moves we add, we should get them all back
    path.moveTo(SK_Scalar1 * 2, SK_Scalar1);
    path.moveTo(SK_Scalar1 * 3, SK_Scalar1 * 2);
    auto iterate = SkPathPriv::Iterate(path);
    auto iter = iterate.begin();
    REPORTER_ASSERT(reporter, iter == iterate.end());
#endif
```

Good:

```cpp
// Good: Update test assertions to explicitly verify the new behavior
    // Consecutive moveTo calls now replace the previous position
    path.moveTo(2, 3);
    auto iterate = SkPathPriv::Iterate(path);
    auto iter = iterate.begin();
    {
        auto [verb, pts, w] = *iter++;
        REPORTER_ASSERT(reporter, verb == SkPathVerb::kMove);
        REPORTER_ASSERT(reporter, pts[0].fX == 2);
        REPORTER_ASSERT(reporter, pts[0].fY == 3);
    }
    REPORTER_ASSERT(reporter, iter == iterate.end());
```
