---
name: skia/tests/use-cts-enforcement-next-release
description: When defining new GPU tests or enabling previously skipped tests for CTS, specify CtsEnforcement::kNextRelease instead of older milestones or kNever
rubric: testing
severity: should_fix
---

**Rule.** When defining new GPU unit tests or enabling previously skipped tests
for Android CTS (such as when resolving a TODO or backend bug), set the CTS
enforcement level to CtsEnforcement::kNextRelease (or CtsEnforcement::kNever
only if the test is permanently excluded from CTS). Do not copy older finalized
CtsEnforcement::kApiLevel_* milestones from existing tests, and do not leave
tests on kNever or kToBeDetermined once backend support is complete.

**Why.** New or newly enabled tests must not retroactively enforce CTS
requirements on older Android vendor API levels where driver support or behavior
may differ. CtsEnforcement::kNextRelease ensures the test is enforced only
starting with the next Android release, after which entries are bulk-migrated to
a finalized kApiLevel_* during milestone branching.

## Examples

Bad:

```cpp
// Bad: Copied older kApiLevel milestone into new test
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SimplifyPaintTest_Graphite,
                                         reporter,
                                         context,
                                         CtsEnforcement::kApiLevel_202404) {
    // ...
}

// Bad: Left on kNever after resolving TODO to enable test in CTS
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(MultisampleRetainTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNever) {
    // ...
}
```

Good:

```cpp
// Good: Uses kNextRelease placeholder for new tests
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(SimplifyPaintTest_Graphite,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNextRelease) {
    // ...
}

// Good: Promoted to kNextRelease when backend support is implemented
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(MultisampleRetainTest,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNextRelease) {
    // ...
}
```
