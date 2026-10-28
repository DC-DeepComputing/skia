---
name: skia/tests/use-reporter-context-for-subtests
description: In unit tests with repetitive assertion blocks or test cases, structure table-driven tests using skiatest::ReporterContext rather than copying assertion boilerplate or formatting strings
rubric: readability
severity: should_fix
---

**Rule.** Consolidate repetitive unit test functions (DEF_TEST) or assertion
blocks that execute identical setup and verification sequences across different
inputs into table-based tests or helper functions. Use a stack-allocated
skiatest::ReporterContext to attach descriptive subtest names, iteration
parameters, file paths, or test descriptions to failure reports, rather than
omitting context, copying assertion boilerplate, or repeatedly formatting
description strings into individual REPORTER_ASSERT calls.

**Why.** Repetitive test functions or assertion sequences duplicate boilerplate
and increase maintenance burden when test logic evolves. Without
skiatest::ReporterContext, assertion failures inside loops, test tables, or
shared helper functions report only line numbers without identifying which input
case failed, while formatting description strings into individual
REPORTER_ASSERT calls clutters assertion expressions and duplicates formatting
across multiple assertions.

## Examples

Bad:

```cpp
// Repetitive assertion boilerplate without subtest context:
SkPoint firstPoint = {5.0, 5.0};
SkColor firstPointColor = pixmap.getColor(firstPoint.x(), firstPoint.y());
REPORTER_ASSERT(reporter, SkColorGetR(firstPointColor) > 0 && SkColorGetR(firstPointColor) < 255);

SkPoint firstPointOffset = {4.0, 5.0};
SkColor firstPointOffsetColor = pixmap.getColor(firstPointOffset.x(), firstPointOffset.y());
REPORTER_ASSERT(reporter, SkColorGetR(firstPointOffsetColor) > 0 && SkColorGetR(firstPointOffsetColor) < 255);
```

Good:

```cpp
// Factored helper lambda with skiatest::ReporterContext:
auto test = [&](const std::string& name, SkPoint pt, SkColor expected) {
    skiatest::ReporterContext subtest(reporter, name);
    SkColor actual = pixmap.getColor((int)pt.x(), (int)pt.y());
    REPORTER_ASSERT(reporter, actual == expected, "%s: %x != %x", name.c_str(), actual, expected);
};

test("above first point", {5.0, 4.0}, SK_ColorWHITE);
test("on first point", {5.0, 5.0}, SK_ColorBLACK);
```
