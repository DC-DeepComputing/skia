---
name: skia/gpu/prefer-power-of-two-for-gpu-thresholds
description: When defining heuristic complexity limits or capacity thresholds in GPU subsystems, use power-of-two bit shifts rather than arbitrary decimal constants
rubric: readability
severity: nit
---

**Rule.** When defining heuristic complexity limits, resource bounds, or
capacity thresholds in GPU subsystems (such as path verb or edge limits for path
renderers), prefer standard power-of-two values expressed as bit shifts (e.g., 1
<< 14 or (1 << 14) - 1) rather than arbitrary decimal constants (e.g., 10240)
derived from individual pathological test cases. Expressing limits as
power-of-two bit shifts signals intentional capacity headroom, conforms to GPU
memory and hardware alignment conventions, and avoids overly restrictive limits
tailored to a single test input.

**Why.** Decimal constants derived solely from an isolated test failure appear
as unexplained magic numbers and risk being prematurely restrictive for valid
complex workloads. Standard power-of-two boundaries expressed via bit shifts
clarify sizing intent, follow GPU subsystem conventions, and provide predictable
exponential headroom.

## Examples

Bad:

```cpp
class PathRenderer : public SkRefCnt {
public:
    // Bad: Arbitrary decimal constant tuned to a single pathological test input
    static constexpr int kMaxGPUPathRendererVerbs = 10240;
};
```

Good:

```cpp
class PathRenderer : public SkRefCnt {
public:
    // Good: Standard power-of-two threshold expressed as a bit shift with headroom
    static constexpr int kMaxGPUPathRendererVerbs = 1 << 14;
};
```
