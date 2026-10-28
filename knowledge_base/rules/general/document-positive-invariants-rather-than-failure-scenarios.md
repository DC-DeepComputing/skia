---
name: skia/document-positive-invariants-rather-than-failure-scenarios
description: When documenting non-obvious invariants or omitted updates, state the positive invariant and downstream assumptions directly instead of narrating speculative failure scenarios
rubric: readability
severity: nit
---

**Rule.** When documenting non-obvious algorithmic invariants, pre-initialized
state, or omitted array/buffer updates, state the concrete affirmative invariant
and downstream assumptions directly. Do not narrate speculative edge cases,
historical bug stories, or dramatic failure consequences (e.g., 'will crash
miserably on an assert').

**Why.** Speculating about edge cases, bug histories, or catastrophic failure
modes obscures the actual state requirement and confuses readers about what the
code guarantees. Clearly documenting the positive invariant (such as a
pre-initialized boundary element) and what downstream code assumes makes the
invariant straightforward to verify, test, and maintain.

## Examples

Bad:

```cpp
// fClusterIndexes is another (long) story. It has n+1 elements but:
// It's possible that fRun has some mismatch between glyph ranges and grapheme cluster edges.
// (for instance, "🇳🇩🇮🇩⃢" is unicode correct, but it's a mess inside)
// For the last codepoint however we should not set the end of the "piece" run to it
// or we will not have full text coverage and will crash miserably on some ASSERT later.
piece->fClusterIndexes[index] = run->fClusterIndexes[i];
```

Good:

```cpp
// fClusterIndexes n+1 is already set to the end of the run.
// Do not attempt to overwrite this value with the cluster index
// that starts the next Run.
// It is assumed later that all clusters in a Run are contained by the Run.
piece->fClusterIndexes[index] = run->fClusterIndexes[i];
```
