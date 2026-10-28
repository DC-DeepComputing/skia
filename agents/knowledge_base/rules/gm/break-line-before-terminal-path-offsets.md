---
name: skia/gm/break-line-before-terminal-path-offsets
description: When chaining compact SkPathBuilder calls in GM tests, keep intrinsic geometry on one line and place terminal transforms like makeOffset on a separate line
rubric: readability
severity: should_fix
---

**Rule.** When constructing paths using compact SkPathBuilder chains in GM
visual tests, keep the intrinsic geometry and .detach() on a single line if it
fits within column limits, rather than breaking each builder method onto a
separate line. Place terminal positional transformations (such as .makeOffset())
on a separate continuation line to distinguish shape definition from canvas
placement.

**Why.** Placing every method call of a short builder chain on its own line
causes unnecessary vertical sprawl for simple test shapes. Conversely, placing
terminal positioning transforms like .makeOffset() on a separate continuation
line clearly distinguishes intrinsic path construction from test layout
coordinates, making canvas placement easier to scan and adjust.

## Examples

Bad:

```cpp
// Bad: Vertically sprawling short builder chain or inlining canvas offset
auto p1 = SkPathBuilder()
                  .lineTo(0, 5)
                  .lineTo(5, 5)
                  .lineTo(5, 0)
                  .detach()
                  .makeOffset(5, 5);

auto p2 = SkPathBuilder().quadTo(15, 5, 0, 10).detach().makeOffset(20, 5);
```

Good:

```cpp
// Good: Compact builder chain on one line, placement offset on continuation line
auto p1 = SkPathBuilder().lineTo(0, 5).lineTo(5, 5).lineTo(5, 0).detach()
                  .makeOffset(5, 5);

auto p2 = SkPathBuilder().quadTo(15, 5, 0, 10).detach()
                  .makeOffset(20, 5);
```
