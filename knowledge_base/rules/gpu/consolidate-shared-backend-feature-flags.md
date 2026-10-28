---
name: skia/gpu/consolidate-shared-backend-feature-flags
description: When configuring shared GPU capability or binding structs, represent features with generic flags and BackendApi rather than per-backend booleans or backend-private fields
rubric: design
severity: should_fix
---

**Rule.** In backend-agnostic GPU structs (such as `Caps` or
`ResourceBindingRequirements`), represent multi-backend capabilities with
generic boolean flags and `BackendApi` rather than proliferating per-backend
flags. Additionally, do not add speculative or backend-private configuration
fields (such as attachment indices used only by a single backend pipeline) until
actively consumed by shared code; reference static backend constants directly in
backend pipelines.

**Why.** Adding per-backend boolean flags or backend-private configuration
fields bloats shared capability abstractions with dead code, complicates
initialization across backends, and bakes in premature assumptions. Keeping
shared structs strictly agnostic and scoped to configuration actively consumed
by shared code decouples feature availability from backend-specific code
generation and keeps backend pipelines self-contained.

## Examples

Bad:

```cpp
// Bad: Multiplying per-backend flags or adding backend-private fields in shared structs.
struct ResourceBindingRequirements {
    bool fUseVulkanPushConstantsForIntrinsicConstants = false;
    bool fUseDawnPushConstantsForIntrinsicConstants = false;
    // Backend-private field unused by shared SkSL generation:
    int fInputAttachmentIdxForLoadingMSAA = kUnassigned;
};
```

Good:

```cpp
// Good: Generic feature flags with BackendApi, omitting backend-private fields.
struct ResourceBindingRequirements {
    BackendApi fBackendApi = BackendApi::kUnsupported;
    bool fUsePushConstantsForIntrinsicConstants = false;
    int fInputAttachmentSetIdx = kUnassigned;
    // Backend pipelines reference static backend constants directly.
};
```
