---
name: skia/gpu/guard-telemetry-hit-counters-to-first-use
description: When collecting cache hit telemetry compared against object creation costs or waste, increment hit counters only on first use rather than on every cache lookup
rubric: correctness
severity: should_fix
---

**Rule.** When recording telemetry counters for cache hit benefits that are
compared against unique object creation costs or purge waste, guard counter
increments so they record only initial usage (e.g. checking !wasUsed()) rather
than firing on every lookup. Do not increment benefit counters unconditionally
on repeated cache hits when comparing against unique-object metrics.

**Why.** Incrementing benefit counters on every cache lookup measures per-draw
event frequency rather than unique object reuse. This inflates the counter scale
relative to object creation and purge counts, distorting telemetry comparisons
and bucket alignment across complementary metrics.

## Examples

Bad:

```cpp
if (!forPrecompile && (*entry)->fromPrecompile()) {
    ++fStats.fNormalPreemptedByPrecompile;
}
```

Good:

```cpp
if (!forPrecompile && (*entry)->fromPrecompile() && !(*entry)->wasUsed()) {
    ++fStats.fNormalPreemptedByPrecompile;
}
```
