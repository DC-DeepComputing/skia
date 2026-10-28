---
name: skia/name-test-forcing-helpers-with-action-and-timing
description: When declaring test-only hooks that prime internal state to trigger edge cases on subsequent operations, name them with forceNext rather than setup
rubric: readability
severity: nit
---

**Rule.** When naming test hooks or simulation helpers that prime internal state
to trigger an edge case, failure, or boundary condition on a subsequent call,
use explicit action verbs and temporal qualifiers (such as `forceNext...`)
instead of ambiguous prefixes like `setup...`. Make the intended side effect and
timing immediately clear to readers at both declaration and test call sites.

**Why.** Method names like `setupEpochOverflow()` or `setupFailure()` are
ambiguous: they suggest general configuration or fixture initialization rather
than actively forcing an impending edge-case state for the immediate next
operation. Naming helpers with `forceNext...` clearly communicates the helper's
purpose, action, and immediate temporal scope.

## Examples

Bad:

```cpp
#if defined(GPU_TEST_UTILS)
// Vague: suggests general configuration rather than priming state for the next call
void setupEpochOverflow();
void setupPurgeFailure();
#endif
```

Good:

```cpp
#if defined(GPU_TEST_UTILS)
// Clear action verb and timing: explicitly primes state for the subsequent call
void forceNextEpochOverflow();
void forceNextPurgeFailure();
#endif
```
