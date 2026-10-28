---
name: skia/gpu/check-plot-token-interval-not-needs-upload
description: When checking DrawAtlas plot availability during compaction or resource deactivation, test flush token intervals directly rather than checking needsUpload
rubric: readability
severity: should_fix
---

**Rule.** When determining whether an atlas plot is in use or available (such as
during compaction, sub-allocation, or GPU resource deactivation in DrawAtlas or
GrDrawOpAtlas), evaluate only plot->lastUseToken().inInterval(...) against the
flush token. Do not check plot->needsUpload(), as plots requiring uploads always
record associated draw tokens.

**Why.** In GPU atlas managers, every plot modified for an upload has an
associated draw recorded in the token stream for that flush. Testing
needsUpload() alongside token interval checks is redundant, conflates data
upload scheduling with draw execution synchronization, and clutters plot
availability logic.

## Examples

Bad:

```cpp
// Redundant needsUpload check when evaluating plot availability
if (plot->needsUpload() || plot->lastUseToken().inInterval(fPrevFlushToken, token)) {
    // Treat plot as in use
}
```

Good:

```cpp
// Evaluate plot availability solely against token intervals
if (plot->lastUseToken().inInterval(fPrevFlushToken, token)) {
    // Treat plot as in use
}
```
