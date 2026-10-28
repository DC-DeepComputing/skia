---
name: skia/tests/document-negative-test-failure-reasons
description: When writing negative test cases for compatibility, validation, or predicate functions, add comments explaining the specific condition or mismatch causing expected failure
rubric: readability
severity: should_fix
---

**Rule.** When authoring negative test cases that assert failure, rejection, or
incompatibility (such as compatibility checks or validation predicates returning
false), add an explanatory comment detailing the specific condition, structural
difference, or invalid input property that causes the operation to fail. Avoid
leaving unannotated negative assertions where the cause of failure must be
reverse-engineered from raw coordinate lists or data setup.

**Why.** When tests assert negative outcomes, the structural difference or
boundary violation causing rejection is rarely self-evident from raw
coordinates, data buffers, or fixture configurations. Explicit comments document
the invariant being tested, make the test suite auditable for reviewers, and
ensure future maintainers understand which specific defect or incompatibility
each test case verifies.

## Examples

Bad:

```cpp
// Variant 1: C++ negative compatibility assertion without explanation
p1.moveTo(0, 0); p1.lineTo(10, 10);
p2.moveTo(0, 0); p2.quadTo(5, 5, 10, 10);
REPORTER_ASSERT(reporter, !p1.isInterpolatable(p2));

// Variant 2: CanvasKit / JS negative predicate assertion without explanation
const p1 = new CanvasKit.PathBuilder().moveTo(0, 2).lineTo(0, 4).detach();
const p2 = new CanvasKit.PathBuilder().moveTo(0, 2).lineTo(0, 4).lineTo(0, 6).detach();
expect(CanvasKit.Path.CanInterpolate(p1, p2)).toBe(false);
```

Good:

```cpp
// Variant 1: Documented verb mismatch preventing interpolation
p1.moveTo(0, 0); p1.lineTo(10, 10);
p2.moveTo(0, 0); p2.quadTo(5, 5, 10, 10);
// p1 contains a line verb whereas p2 contains a quad verb; verb mismatch prevents interpolation.
REPORTER_ASSERT(reporter, !p1.isInterpolatable(p2));

// Variant 2: Documented point/verb count mismatch
const p1 = new CanvasKit.PathBuilder().moveTo(0, 2).lineTo(0, 4).detach();
const p2 = new CanvasKit.PathBuilder().moveTo(0, 2).lineTo(0, 4).lineTo(0, 6).detach();
// p1 has 2 points while p2 has 3 points; mismatched point counts prevent interpolation.
expect(CanvasKit.Path.CanInterpolate(p1, p2)).toBe(false);
```
