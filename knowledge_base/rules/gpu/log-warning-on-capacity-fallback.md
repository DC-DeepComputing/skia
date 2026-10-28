---
name: skia/gpu/log-warning-on-capacity-fallback
description: When client configurations or registrations exceed fixed capacity limits and gracefully fall back, emit a warning with SKIA_LOG_W instead of silently dropping items
rubric: readability
severity: should_fix
---

**Rule.** When client-provided configurations or registrations exceed a fixed
capacity limit and gracefully degrade to a generic fallback instead of failing,
emit an informative warning log using SKIA_LOG_W (or SKGPU_LOG_W). Report the
limit and the count of accepted versus registered items rather than silently
ignoring or dropping surplus items.

**Why.** Silently dropping surplus configurations or falling back to
un-optimized generic paths leaves developers unaware of why features (such as
stably keyed pipeline precompilation) are not active for all inputs. Emitting a
warning log alerts developers to the capacity constraint and diagnoses
unexpected performance regressions while allowing execution to proceed safely.

## Examples

Bad:

```cpp
if (fUserDefinedKnownCodeSnippets.size() >= kUserDefinedKnownRuntimeEffectsReservedCnt) {
    // too many user-defined known runtime effects
    return;
}
```

Good:

```cpp
if (fUserDefinedKnownCodeSnippets.size() >= kUserDefinedKnownRuntimeEffectsReservedCnt) {
    SKIA_LOG_W("Too many user-defined known runtime effects. Only %d out of %zu "
               "will be known.\n",
               kUserDefinedKnownRuntimeEffectsReservedCnt,
               userDefinedKnownRuntimeEffects.size());
    // too many user-defined known runtime effects
    return;
}
```
