---
name: skia/tests/preserve-path-fill-type-with-static-factories
description: When migrating paths from mutable SkPath to static shape factories like SkPath::Circle or SkPath::Oval, preserve non-default SkPathFillType using .makeFillType() rather than resetting to default kWinding
rubric: testing
severity: should_fix
---

**Rule.** When replacing mutable SkPath construction (where attributes like
fillType were set on a shared or reused path variable) with static shape
factories (such as SkPath::Circle, SkPath::Oval, or SkPath::Rect), explicitly
chain .makeFillType(...) to preserve non-default fill types such as
SkPathFillType::kEvenOdd. Do not rely on static factories to retain prior
instance configuration, as they construct fresh paths with default kWinding fill
type.

**Why.** Static path factories construct brand-new SkPath instances that default
to SkPathFillType::kWinding. When sequential test blocks or functions rely on a
previously set fill type (such as kEvenOdd) to test conversion algorithms or
render rules, reassigning the variable using a bare factory silently discards
the configured fill type, causing tests to lose coverage or pass vacuously.

## Examples

Bad:

```cpp
// Bad: Switching to a static factory silently resets fillType to default kWinding,
// losing the kEvenOdd fill type needed to exercise AsWinding conversion.
test.setFillType(SkPathFillType::kEvenOdd);
// ... earlier test assertions ...
test = SkPath::Circle(5, 5, 10);
auto result = AsWinding(test);
```

Good:

```cpp
// Good: Explicitly chain .makeFillType() to preserve the intended fill type.
test.setFillType(SkPathFillType::kEvenOdd);
// ... earlier test assertions ...
test = SkPath::Circle(5, 5, 10).makeFillType(SkPathFillType::kEvenOdd);
auto result = AsWinding(test);
```
