---
name: skia/gpu/document-deferred-mutation-properties-on-captured-pointers
description: When documenting member variables or raw pointers retained for deferred mutation, enumerate all properties modified through the pointer rather than describing an incomplete subset
rubric: readability
severity: nit
---

**Rule.** When documenting member variables, out-pointers, or captured pointers
stored for deferred mutation (such as deferred draw or clip parameters),
enumerate all state, fields, or properties that are modified through the pointer
(e.g., both bounds updates and deferred painter's order or depth assignment). Do
not describe only an incomplete subset of the deferred modifications in the
member comment.

**Why.** Omitting key deferred mutations from member documentation obscures the
full lifecycle and side effects of captured pointers across rendering phases,
misleading maintainers about when and how target state (such as draw order or
depth) is modified.

## Examples

Bad:

```cpp
// fCaptureParams: Points to the drawParams of the depth-only draw. Allows the initial
//                 coarse bounds (recorded in drawClipImmediate) to be updated to tighter
//                 bounds later.
DrawParams* fCaptureParams;
```

Good:

```cpp
// fCaptureParams: Points to the drawParams of the depth-only draw. Allows: 1) The initial
//                 coarse bounds (recorded in drawClipImmediate) to be updated to tighter
//                 bounds later. 2) Deferred assignment of the Z value the draw will use.
DrawParams* fCaptureParams;
```
