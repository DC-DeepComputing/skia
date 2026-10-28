---
name: skia/gpu/assert-backend-invariants-locally
description: When dispatching on backend types like BackendApi, branch on backend identity alone and assert capability invariants locally rather than checking them in the condition
rubric: readability
severity: should_fix
---

**Rule.** Do not include invariant backend capabilities in `if`/`else if` branch
conditions when dispatching on backend types (such as `BackendApi`). Branch
strictly on backend identity alone, and verify assumed capabilities or
configurations with `SkASSERT` locally within the branch where they are relied
upon.

**Why.** Combining invariant backend requirements with backend type checks (e.g.
`backend == BackendApi::kDawn && separateSamplers`) misleads readers into
thinking that a backend lacking that capability is a valid configuration that
should fall through to fallback branches. Branching on backend identity alone
and asserting invariants locally enforces preconditions right at the point of
use and prevents silent fallthrough bugs.

## Examples

Bad:

```cpp
} else if (backend == BackendApi::kDawn && separateSamplers) {
    // Relies on Dawn using separate samplers.
    SkSL::String::appendf(
        &sksl, "layout(webgpu, sampler=%d, texture=%d) ", index, index + 1);
    index += 2;
}
```

Good:

```cpp
} else if (backend == BackendApi::kDawn) {
    // Dawn is expected to use separate texture/samplers.
    SkASSERT(separateSamplers && !texturesUseDistinctIdxRanges);
    SkSL::String::appendf(
        &sksl, "layout(webgpu, sampler=%d, texture=%d) ", index, index + 1);
    index += 2;
}
```
