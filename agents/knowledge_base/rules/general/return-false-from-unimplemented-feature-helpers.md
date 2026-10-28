---
name: skia/return-false-from-unimplemented-feature-helpers
description: When scaffolding placeholder or helper functions for upcoming unimplemented features, return false or no-op defaults with a TODO instead of speculative condition checks
rubric: readability
severity: should_fix
---

**Rule.** When scaffolding placeholder or helper functions for upcoming,
unimplemented features, explicitly return false (or the safe default no-op
value) accompanied by an explanatory TODO comment. Do not evaluate speculative
conditional checks ahead of time before the underlying capability or pipeline is
implemented.

**Why.** Writing premature conditional logic for an unsupported feature misleads
callers and reviewers into believing the capability is functional. If runtime
parameters satisfy the speculative conditions, callers may trigger incomplete
code paths that fail; returning false keeps the unsupported status explicit and
safe until full implementation lands.

## Examples

Bad:

```cpp
// Bad: evaluates speculative conditions for an unsupported feature
bool RenderPassDescWillImplicitlyLoadMSAA(const RenderPassDesc& renderPassDesc) {
    // TODO: This is currently not possible, but will be with an upcoming change...
    return renderPassDesc.fColorAttachment.fSampleCount == 1 && renderPassDesc.fSampleCount > 1;
}
```

Good:

```cpp
// Good: explicitly returns false until the feature is actually implemented
bool RenderPassDescWillImplicitlyLoadMSAA(const RenderPassDesc& renderPassDesc) {
    // TODO: This is currently not possible, but will be with an upcoming change...
    // For now, simply return false.
    return false;
}
```
