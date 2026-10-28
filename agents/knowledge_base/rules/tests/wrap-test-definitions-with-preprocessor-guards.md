---
name: skia/tests/wrap-test-definitions-with-preprocessor-guards
description: When conditionally compiling or disabling unit tests for specific configurations, wrap the entire DEF_TEST definition and document divergence rationale with a tracking TODO
rubric: readability
severity: should_fix
---

**Rule.** When conditionally enabling or disabling a unit test based on build
configuration (such as SK_DEBUG, target architecture, or feature defines), wrap
the entire DEF_TEST macro definition in the preprocessor directive rather than
placing guards inside the function body. Additionally, whenever disabling a test
or altering tolerances under specific build configurations (such as #ifndef
SK_DEBUG) to bypass known failures or debug assertions, document the rationale
explaining why the divergence is acceptable and include a tracking TODO(bug)
with an issue tracker ID.

**Why.** Placing preprocessor guards inside the test function body leaves an
empty stub registered with skiatest::TestRegistry that falsely passes without
running assertions. Furthermore, disabling tests or altering tolerances across
build configurations (e.g. Debug vs. Release) without explanation obscures
failures and appears as an unmotivated workaround; documenting the rationale and
adding an attributed tracking TODO ensures known defects remain visible for
future resolution.

## Examples

Bad:

```cpp
// Bad: Preprocessor guards inside test body leave an empty stub; unexplained configuration hacks obscure failures.
DEF_TEST(Triangulator_bug421959607, r) {
#ifndef SK_DEBUG
    int vertexCount = GrAATriangulator::PathToAATriangles(
            path, GrPathUtils::kDefaultTolerance, clipBounds, &alloc);
    REPORTER_ASSERT(r, vertexCount);
#endif
}

DEF_TEST(Triangulator_ToleranceHack, r) {
    auto tolerance = GrPathUtils::kDefaultTolerance;
#ifdef SK_DEBUG
    tolerance = .19f; // Hack to get debug builds to pass without tracking bug or explanation
#endif
    REPORTER_ASSERT(r, GrAATriangulator::PathToAATriangles(path, tolerance, clipBounds, &alloc));
}
```

Good:

```cpp
// Good: Wrap the entire DEF_TEST definition and document the divergence rationale with a tracking TODO.
// TODO: b/425655494 - Reproduces a fuzzer nullptr dereference on Release builds; disabled on
// Debug builds because it triggers ordering asserts in validate_edge_list().
#ifndef SK_DEBUG
DEF_TEST(Triangulator_bug421959607, r) {
    int vertexCount = GrAATriangulator::PathToAATriangles(
            path, GrPathUtils::kDefaultTolerance, clipBounds, &alloc);
    REPORTER_ASSERT(r, vertexCount);
}
#endif
```
