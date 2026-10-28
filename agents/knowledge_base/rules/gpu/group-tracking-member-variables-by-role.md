---
name: skia/gpu/group-tracking-member-variables-by-role
description: When declaring member variables in stateful manager or tracking classes, group related fields together by functional role or lifecycle rather than interleaving them arbitrarily
rubric: readability
severity: should_fix
---

**Rule.** When declaring member variables in classes that track multi-phase
state, offsets, or alignments, group related fields together by functional role,
subsystem, or lifecycle phase. Avoid interleaving unrelated tracking state (such
as phase checkpoints, rewind marks, and progressive alignments), and separate
distinct functional groups with blank lines or explanatory comments.

**Why.** Interleaving member variables from different mechanisms or recording
phases obscures their dependencies and makes class state difficult to reason
about and maintain. Grouping related fields together clarifies which variables
change together and helps prevent state desynchronization across lifecycle
operations.

## Examples

Bad:

```cpp
class UniformManager {
    Layout fLayout;
    int fRewindOffset = 0;
    int fReqAlignment = 1;
    int fEndPaintOffset = 0;
    int fStructBaseAlignment = 0;
    int fEndPaintAlignment = 1;
};
```

Good:

```cpp
class UniformManager {
    Layout fLayout;

    // Progressive and structural alignment requirements
    int fReqAlignment = 1;
    int fStructBaseAlignment = 0;

    // Phase boundary checkpoints and offsets
    int fEndPaintOffset = 0;
    int fEndPaintAlignment = 1;
};
```
