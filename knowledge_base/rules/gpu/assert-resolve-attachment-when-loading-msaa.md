---
name: skia/gpu/assert-resolve-attachment-when-loading-msaa
description: When asserting or validating Graphite RenderPassDesc resolve attachments, ensure single-sampled color attachments have no resolve attachment and MSAA-load subpasses require one
rubric: correctness
severity: should_fix
---

**Rule.** When validating, asserting, or configuring Graphite RenderPassDesc
attachments, enforce resolve attachment invariants: if the color attachment is
single-sampled (fColorAttachment.fSampleCount == 1),
fColorResolveAttachment.fFormat must be TextureFormat::kUnsupported, as resolve
attachments only exist to resolve multisampled attachments. Conversely, when
configuring render passes or subpasses that load MSAA from resolve (such as
needLoadMSAAFromResolveSubpass), assert that fColorResolveAttachment is valid
with a supported format.

**Why.** In Graphite, resolve attachments exist solely to downsample
multisampled color attachments, so expecting or allowing a resolve attachment on
single-sampled color passes violates RenderPassDesc invariants and corrupts
backend pass setup. Conversely, subpass configurations loading MSAA from resolve
strictly depend on that attachment being present; omitting validation produces
invalid attachment references (such as VK_ATTACHMENT_UNUSED) or broken subpass
dependency chains.

## Examples

Bad:

```cpp
// Assuming single-sampled color attachment can have a resolve attachment:
SkASSERT(renderPassDesc.fColorAttachment.fSampleCount > 1);
if (renderPassDesc.fColorResolveAttachment.fFormat != TextureFormat::kUnsupported) { ... }

// Missing resolve attachment validation when loading MSAA from resolve:
void populate_attachment_refs(const RenderPassDesc& renderPassDesc,
                              bool needLoadMSAAFromResolveSubpass, ...) {
    SkASSERT(renderPassDesc.fColorAttachment.fFormat != TextureFormat::kUnsupported);
    // Missing check that resolve attachment is valid when loading MSAA
}
```

Good:

```cpp
// Asserting single-sampled color attachments have no resolve attachment:
SkASSERT(renderPassDesc.fColorAttachment.fSampleCount > 1 ||
         renderPassDesc.fColorResolveAttachment.fFormat == TextureFormat::kUnsupported);

// Asserting resolve attachment is present when loading MSAA from resolve:
void populate_attachment_refs(const RenderPassDesc& renderPassDesc,
                              bool needLoadMSAAFromResolveSubpass, ...) {
    SkASSERT(renderPassDesc.fColorAttachment.fFormat != TextureFormat::kUnsupported);
    SkASSERT(!needLoadMSAAFromResolveSubpass ||
             renderPassDesc.fColorResolveAttachment.fFormat != TextureFormat::kUnsupported);
}
```
