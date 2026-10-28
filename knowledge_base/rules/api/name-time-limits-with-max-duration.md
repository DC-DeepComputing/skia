---
name: skia/api/name-time-limits-with-max-duration
description: When naming time limit, timeout, or duration budget parameters, use standard conceptual terms like maxDuration or maxActionDur rather than trigger-based phrases like quitDur or stopDur
rubric: readability
severity: nit
---

**Rule.** When declaring parameters for time limits, timeouts, or duration
budgets, name them using standard conceptual terms (such as maxDuration or
max<Action>Dur, e.g. maxPurgingDur) rather than trigger-based verb phrases like
quit...Dur or stop...Dur. If the implementation checks the budget heuristically
between iterations and can slightly surpass the threshold, explain the exact
boundary semantics in doc comments rather than embedding execution mechanics in
the parameter name.

**Why.** Trigger-based verb phrases like quitPurgingDur or stopDuration are
awkward, expose internal loop break mechanics, and obscure caller intent.
Standard conceptual names like maxDuration or maxPurgingDur keep call sites
intuitive and uniform, while boundary nuances are properly documented in
accompanying comments.

## Examples

Bad:

```cpp
// Parameter named after internal loop-exit trigger rather than conceptual limit:
void performDeferredCleanup(
        std::chrono::milliseconds msNotUsed,
        std::optional<std::chrono::microseconds> microsQuitPurgingDur = std::nullopt);
```

Good:

```cpp
/**
 * @param msNotUsed Unlocked resources unused for this duration will be cleaned up.
 * @param microsMaxPurgingDur Maximum duration spent purging; checked between iterations.
 */
void performDeferredCleanup(
        std::chrono::milliseconds msNotUsed,
        std::optional<std::chrono::microseconds> microsMaxPurgingDur = std::nullopt);
```
