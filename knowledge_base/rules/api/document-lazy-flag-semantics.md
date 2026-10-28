---
name: skia/api/document-lazy-flag-semantics
description: When documenting public APIs, cache limits, or option flags, accurately describe evaluation timing and side effects, clarifying deferred enforcement or gated invocation over immediate execution
rubric: readability
severity: should_fix
---

**Rule.** When documenting public API methods, cache limits, or configuration
flags, ensure doc comments accurately describe actual side-effect timing and
evaluation guarantees rather than implying immediate execution. Clarify whether
actions are deferred (such as cache limits taking effect only on subsequent
mutations) or require explicit invocations (such as boolean flags enabling
separate control APIs), especially when adapting comments from similar eager
APIs.

**Why.** Doc comments that overstate immediacy or copy eager side effects from
sibling APIs mislead callers about when resource management, cache invalidation,
or capture occurs. Accurate evaluation timing prevents false assumptions about
performance overhead, automatic purging, and operational state.

## Examples

Bad:

```cpp
// 1. Misstating lazy cache limit enforcement as eager purging (copied from sibling API):
/**
 *  Set the limit to the number of entries in the typeface cache, and return
 *  the previous value. If this new value is lower than the previous,
 *  it will automatically try to purge entries to meet the new limit.
 */
static int SetTypefaceCacheCountLimit(int count);

// 2. Implying immediate execution on a flag that merely enables separate APIs:
/**
 * Capture all draw calls and surface creation from Recorders spawned from the Context.
 * Enables the use of startCapture and endCapture functions.
 */
bool fEnableCapture = false;
```

Good:

```cpp
// 1. Accurately documenting deferred evaluation on mutation:
/**
 *  Set the limit to the number of entries in the typeface cache, and return
 *  the previous value. Changes to this only take effect the next time
 *  each cache object is modified.
 */
static int SetTypefaceCacheCountLimit(int count);

// 2. Explicitly stating the flag enables APIs and actions occur upon invocation:
/**
 * Enables the use of startCapture and endCapture functions. Calling these APIs will capture
 * all draw calls and surface creation from Recorders spawned from the Context.
 */
bool fEnableCapture = false;
```
