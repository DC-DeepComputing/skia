---
name: skia/avoid-inaccurate-ctm-equivalence-claims
description: When documenting coordinate transformations or approximations between local and device space, do not claim identical results across arbitrary CTMs when equivalence only holds under translation
rubric: readability
severity: should_fix
---

**Rule.** When documenting coordinate-space transformations or approximations
between local and device space, do not make inaccurate claims of equivalence
across arbitrary transformations (such as claiming behavior is identical
regardless of CTM when it only holds under translation). Explicitly state the
exact matrix conditions required for the approximation to hold, or omit
overbroad equivalence claims.

**Why.** Local coordinates are mapped to device coordinates through the CTM,
meaning local-space offsets or adjustments only match device space under
identity or pure translation. Inaccurate comments claiming identical results
regardless of CTM mislead maintainers about transformation invariance and shader
or geometry behavior under scaling and rotation.

## Examples

Bad:

```cpp
// To better approximate past behavior, we add 0.5 to the coordinates here. This is _not_ the same because
// this adjustment is occurring in local space, not device space, but it means that the "same"
// noise will be calculated regardless of CTM.
half2 noiseVec = half2((coords + 0.5) * baseFrequency);
```

Good:

```cpp
// To better approximate past behavior, we add 0.5 to the coordinates here. This is _not_ exactly the same
// because this adjustment is occurring in local space, not device space.
half2 noiseVec = half2((coords + 0.5) * baseFrequency);
```
