---
name: skia/general-prefer-fluent-skpathbuilder
description: When creating SkPath instances, use dedicated shape factories for standard primitives; for custom paths, chain SkPathBuilder fluently to detach()
rubric: safety
severity: should_fix
---

**Rule.** When creating SkPath instances from standard geometric shapes (such as
rectangles, ovals, round rects, polygons, or lines), use dedicated shape
factories (SkPath::Rect, SkPath::Oval, etc., or SkPathData shape factories
internally) rather than SkPathBuilder().add...().detach(). For custom multi-verb
paths, chain SkPathBuilder methods fluently in a single expression terminating
with .detach(), avoiding mutable local builder variables across statements.

**Why.** Dedicated shape factories preallocate exact storage in one step and
compute shape bounds and convexity directly, avoiding intermediate buffer
reallocations and builder post-processing overhead. For custom paths, fluent
chaining eliminates redundant mutable builder state, minimizes boilerplate, and
allows constructing const paths inline.

## Examples

Bad:

```cpp
// Bad: Using SkPathBuilder for standard shapes incurs intermediate allocations
SkPath path = SkPathBuilder().addOval(r, dir, startIndex).detach();

// Bad: Declaring intermediate mutable builder across multiple mutating statements
SkPathBuilder builder;
builder.moveTo(0, 0).lineTo(100, 0).lineTo(100, 100).close();
SkPath custom = builder.detach();
```

Good:

```cpp
// Good: Dedicated one-call factory preallocates exact storage and metadata
SkPath path = SkPath::Oval(r, dir, startIndex); // or SkPathData::Oval(...) internally

// Good: Fluent chaining directly to detach() without mutable builder variables
const SkPath custom = SkPathBuilder()
                              .moveTo(0, 0)
                              .lineTo(100, 0)
                              .lineTo(100, 100)
                              .close()
                              .detach();
```
