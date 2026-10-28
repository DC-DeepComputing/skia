---
name: skia/gpu/document-subpass-scope-for-attachment-flags
description: When flattening multi-subpass attachment flags into top-level render pass or pipeline metadata structs, document which subpass the flags configure
rubric: readability
severity: should_fix
---

**Rule.** When defining or modifying top-level attachment flags (such as
`fHasColorAttachment`, `fHasColorResolveAttachment`, or `fHasInputAttachment`)
in multi-subpass render pass or pipeline metadata structs, document which
subpass the flags apply to. Clarify whether the flags configure the main drawing
subpass, load-from-resolve subpasses, or every subpass in the pass.

**Why.** In multi-subpass render passes, different subpasses have distinct
attachment configurations (e.g., resolving MSAA before main drawing). Top-level
boolean attachment flags without clarifying comments are ambiguous as to whether
they describe all subpasses, any subpass, or a specific subpass, leading to
incorrect assumptions when configuring pipelines or subpass dependencies.

## Examples

Bad:

```cpp
struct RenderPassFlags {
    bool fHasColorAttachment;
    bool fHasColorResolveAttachment;
    bool fHasDepthStencilAttachment;
    bool fHasInputAttachment;
};
```

Good:

```cpp
struct RenderPassFlags {
    // These attachment flags apply to the main subpass (not the preliminary
    // subpass used when loading MSAA from resolve).
    bool fHasColorAttachment;
    bool fHasColorResolveAttachment;
    bool fHasDepthStencilAttachment;
    bool fHasInputAttachment;
};
```
