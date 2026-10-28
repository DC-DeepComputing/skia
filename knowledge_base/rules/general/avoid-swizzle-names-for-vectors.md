---
name: skia/avoid-swizzle-names-for-vectors
description: When declaring vector or point variables, use semantic names like pos or offset rather than swizzle aliases like xy to avoid stuttering xy.x access
rubric: readability
severity: nit
---

**Rule.** Do not name vector, point, or coordinate variables after their
component swizzles (such as 'xy' or 'xyz') when their members are subsequently
accessed as '.x', '.y', or '.z'. Use a descriptive domain identifier reflecting
the value's role—such as 'pos', 'coord', 'pt', 'delta', or 'offset'—to avoid
repetitive and awkward stuttering expressions like 'xy.x'.

**Why.** Naming a multi-component vector after its swizzle components ('xy')
produces repetitive, stuttering member accesses like 'xy.x' and 'xy.y' that
obscure what the coordinate or offset represents. Giving variables semantic
names clarifies the coordinate's role and makes component accesses
self-documenting.

## Examples

Bad:

```cpp
// Bad: Naming 2D vector 'xy' causes stuttering 'xy.x' / 'xy.y' member accesses
half2 xy = max(half2(rect.xy - coords), half2(coords - rect.zw));
xCoverage = sample(integral, float2(invSixSigma * xy.x, 0.5)).r;
yCoverage = sample(integral, float2(invSixSigma * xy.y, 0.5)).r;
```

Good:

```cpp
// Good: Descriptive identifier 'pos' or 'coord' makes component accesses readable
half2 pos = max(half2(rect.LT - coords), half2(coords - rect.RB));
xCoverage = sample(integral, float2(invSixSigma * pos.x, 0.5)).r;
yCoverage = sample(integral, float2(invSixSigma * pos.y, 0.5)).r;
```
