---
name: skia/api/push-deadline-calculation-to-provider
description: When defining or delegating timed GPU cleanup operations with duration budgets, specify limits in std::chrono::microseconds and compute deadlines inside the shared provider
rubric: safety
severity: should_fix
---

**Rule.** When defining APIs with execution time limits or budgets for
interactive GPU tasks (such as deferred cleanup or purging), specify duration
budgets using microsecond resolution (std::chrono::microseconds) rather than
milliseconds so callers can enforce sub-millisecond limits without truncation.
When delegating timed operations across facade callers (such as Context and
Recorder), forward the duration budget directly and compute absolute deadline
time points inside the shared provider instead of querying clocks and
calculating deadlines in individual callers.

**Why.** Sub-millisecond budgets (e.g., 250 microseconds) are common in
real-time rendering where frame times are tightly constrained (8–16 ms);
requiring milliseconds forces a 1 ms minimum or truncates budgets, risking UI
jank. Computing deadlines in the shared provider avoids duplicating clock
queries and duration arithmetic across multiple facade callers while evaluating
the deadline closer to actual execution.

## Examples

Bad:

```cpp
// Bad: Millisecond budget prevents sub-millisecond limits, and callers duplicate deadline math:
void Context::performDeferredCleanup(
        std::chrono::milliseconds msNotUsed,
        std::optional<std::chrono::milliseconds> stopPurgingPastDuration) {
    auto purgeTime = skgpu::StdSteadyClock::now() - msNotUsed;
    std::optional<StdSteadyClock::time_point> stopPurgingTime;
    if (stopPurgingPastDuration.has_value()) {
        stopPurgingTime = { skgpu::StdSteadyClock::now() + stopPurgingPastDuration.value() };
    }
    fResourceProvider->purgeResourcesNotUsedSince(purgeTime, stopPurgingTime);
}
```

Good:

```cpp
// Good: Microsecond budget supports tight frame budgets; provider computes deadline centrally:
void Context::performDeferredCleanup(
        std::chrono::milliseconds msNotUsed,
        std::optional<std::chrono::microseconds> microsMaxPurgingDur) {
    auto purgeTime = skgpu::StdSteadyClock::now() - msNotUsed;
    fResourceProvider->purgeResourcesNotUsedSince(purgeTime, microsMaxPurgingDur);
}

void ResourceProvider::purgeResourcesNotUsedSince(
        StdSteadyClock::time_point purgeTime,
        std::optional<std::chrono::microseconds> microsMaxPurgingDur) {
    std::optional<StdSteadyClock::time_point> quitPurgingTime;
    if (microsMaxPurgingDur.has_value()) {
        quitPurgingTime = { StdSteadyClock::now() + microsMaxPurgingDur.value() };
    }
    fResourceCache->purgeResourcesNotUsedSince(purgeTime, quitPurgingTime);
}
```
