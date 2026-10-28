---
name: skia/tests/capture-test-context-in-local-lambda
description: When asserting repeatedly against invariant local test fixtures (such as Reporter*, SkPixmap, or SkCanvas), capture shared context in a local lambda rather than repeating arguments
rubric: design
severity: nit
---

**Rule.** When writing repetitive test assertions that inspect shared local
state (such as skiatest::Reporter*, SkPixmap, or SkCanvas), factor the
verification logic into a local helper lambda capturing that invariant context
by reference ([&]). Do not repeatedly pass invariant test fixtures and reporter
pointers to every assertion call site; pass only the variable inputs,
coordinates, and expected outcomes to the local lambda.

**Why.** Plumbing identical context arguments (such as reporter, pixmap, or
canvas) through dozens of test assertions adds visual noise and boilerplate.
Capturing invariant local state in a concise helper lambda isolates repetitive
dispatch logic, highlights the specific inputs and expected values that vary
between test cases, and simplifies future assertion updates.

## Examples

Bad:

```cpp
// Redundantly passing identical local fixtures to every assertion call
InspectPixelColorAtPoint(reporter, pixmap, "above apex", {5, 4}, SK_ColorWHITE);
InspectPixelColorAtPoint(reporter, pixmap, "left of apex", {4, 5}, SK_ColorWHITE);
InspectPixelColorAtPoint(reporter, pixmap, "on apex", {5, 5}, SK_ColorBLACK);
```

Good:

```cpp
// Capture invariant test fixtures in a local lambda by reference
auto assert_color = [&](const std::string& name, SkIPoint pt, SkColor expected) {
    InspectPixelColorAtPoint(reporter, pixmap, name, pt, expected);
};
assert_color("above apex", {5, 4}, SK_ColorWHITE);
assert_color("left of apex", {4, 5}, SK_ColorWHITE);
assert_color("on apex", {5, 5}, SK_ColorBLACK);
```
