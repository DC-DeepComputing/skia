---
name: skia/gpu/document-prepopulated-cache-concurrency
description: When pre-populating shared lookup tables at construction to eliminate locks, document the concurrency model, why lookups assert success, and how to handle new entries
rubric: readability
severity: nit
---

**Rule.** When achieving thread safety by pre-populating a shared lookup table
or cache at construction time rather than lazily synchronizing with locks,
document that concurrency model and invariant at the lookup site in an inline
comment. Explain that eager pre-population eliminates concurrency concerns, why
the lookup is asserted to succeed (e.g., via SkAssertResult or SkASSERT), and
how maintainers should handle new or unhandled configurations.

**Why.** An unsynchronized lookup on a shared data structure followed by an
assertion that the result is non-null appears brittle or unsafe to readers who
cannot tell whether concurrent modification is possible. Clarifying the eager
initialization design in an inline comment confirms the lock-free concurrency
model (immutability after initialization) and prevents maintainers from either
adding redundant locks or failing to register new configurations during
initialization.

## Examples

Bad:

```cpp
sk_cfp<id<MTLDepthStencilState>>* depthStencilState;
depthStencilState = fDepthStencilStates.find(depthStencilSettings);
SkAssertResult(depthStencilState);
return *depthStencilState;
```

Good:

```cpp
sk_cfp<id<MTLDepthStencilState>>* depthStencilState;
depthStencilState = fDepthStencilStates.find(depthStencilSettings);

// Explicitly initialized fDepthStencilStates with all common settings in the ctor,
// freeing us from concurrency concerns (e.g., lazily creating and storing in a map).
// If a new setting is needed, add its initialization to the ctor or reconsider this approach.
SkAssertResult(depthStencilState);
return *depthStencilState;
```
