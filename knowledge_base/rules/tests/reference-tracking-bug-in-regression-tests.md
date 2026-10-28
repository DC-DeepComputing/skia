---
name: skia/tests/reference-tracking-bug-in-regression-tests
description: When authoring regression unit tests or GMs reproducing bugs or fuzzer crashes, document the tracking bug URL or issue ID immediately above the test definition
rubric: readability
severity: nit
---

**Rule.** When writing a regression unit test (such as DEF_TEST,
DEF_GANESH_TEST, or DEF_GRAPHITE_TEST) or Golden Master test (DEF_GM or
DEF_SIMPLE_GM) reproducing a defect or fuzzer crash, include a comment
referencing the tracking bug URL or issue ID (e.g., '// crbug.com/<id>', '//
https://issues.skia.org/issues/<id>', or '// b/<id>') immediately above the test
definition. Alternatively, if the test is named directly after the issue ID
(such as 'DEF_TEST(bug_<id>, ...)' or 'DEF_SIMPLE_GM(crbug_<id>, ...)'), ensure
the issue identifier is clear and traceable.

**Why.** Attaching the issue tracker reference or URL directly to regression
tests links the reproduction case to original crash reports, fuzzer artifacts,
stack traces, and triage history. This preserves essential context for future
maintainers investigating test failures, minimizing regressions, or refactoring
the affected subsystem.

## Examples

Bad:

```cpp
// Reproduction test for a fuzzer-reported concave path crash without tracking context
DEF_TEST(path_walk_edges_concave_large_dx, r) {
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1000, 1000));
    SkPath path = SkPathBuilder().moveTo(0, 0).lineTo(1000, 500).detach();
    surface->getCanvas()->drawPath(path, SkPaint());
}
```

Good:

```cpp
// crbug.com/398075927
DEF_TEST(path_walk_edges_concave_large_dx, r) {
    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(900, 700));
    SkPath path = SkPathBuilder().moveTo(0, 0).lineTo(900, 350).detach();
    surface->getCanvas()->drawPath(path, SkPaint());
}

// Or naming the test after the tracking issue:
// DEF_SIMPLE_GM(crbug_404394639, canvas, 500, 500) { ... }
```
