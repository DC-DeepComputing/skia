---
name: skia/gpu/unify-vulkan-compatible-render-passes
description: When caching Vulkan render passes or generating pipeline keys, centralize key packing in VulkanRenderPass and canonicalize compatible descriptors via shared utilities
rubric: design
severity: should_fix
---

**Rule.** When generating Vulkan render passes or pipeline keys, canonicalize
descriptor compatibility (such as resetting load/store ops) via
MakePipelineCompatibleRenderPass rather than duplicating manual overrides across
resource creation and key generation. Centralize render pass descriptor key
packing and extraction in VulkanRenderPass (via GetRenderPassKey and
ExtractRenderPassDesc) rather than duplicating bit-packing layouts in
VulkanCaps. Unify compatible render passes used for graphics pipeline creation
so input attachments and subpass dependencies are unconditionally configured to
maximize pipeline sharing, while differentiating cache keys for full execution
render passes when destination reads are not required.

**Why.** Duplicating descriptor overrides or bit-packing layouts across
VulkanResourceProvider and VulkanCaps causes desynchronization bugs and code
bloat. Centralizing descriptor compatibility normalization and key packing in
VulkanRenderPass ensures identical compatibility semantics for pipeline creation
while allowing full render passes to preserve optimal attachment layouts without
unnecessary fallbacks to VK_IMAGE_LAYOUT_GENERAL.

## Examples

Bad:

```cpp
// 1. Duplicating bit-packing layout in VulkanCaps:
static constexpr uint32_t kColorFormatOffset = 0;
static constexpr uint32_t kColorNumSamplesOffset = kColorFormatOffset + kFormatBits;
uint32_t VulkanCaps::GetRenderPassDescKeyForPipeline(const RenderPassDesc& rpDesc) {
    return (static_cast<uint32_t>(rpDesc.fColorAttachment.fFormat) << kColorFormatOffset) | ...;
}

// 2. Duplicating descriptor overrides in place:
if (compatibleOnly) {
    finalRenderPassDesc.fColorAttachment.fLoadOp = LoadOp::kDiscard;
    finalRenderPassDesc.fColorAttachment.fStoreOp = StoreOp::kDiscard;
}

// 3. Differentiating compatible render passes based on dst read requirement:
attachmentIdxKeyInfo |= (rpData.fHasInputAttachment
        ? VulkanRenderPass::kColorAttachmentIdx : kAttachmentUnused) << 24;
```

Good:

```cpp
// 1. Centralize key packing and extraction in VulkanRenderPass:
builder[kPipelineKeyRenderPassDescIndex] = VulkanRenderPass::GetRenderPassKey(
        renderPassDesc, /*compatibleForPipelineKey=*/true);
VulkanRenderPass::ExtractRenderPassDesc(rpDescBits, swizzle, strategy, renderPassDesc);

// 2. Canonicalize compatible descriptors via a shared utility:
const RenderPassDesc renderPassDesc = compatibleOnly
        ? MakePipelineCompatibleRenderPass(originalRenderPassDesc)
        : originalRenderPassDesc;

// 3. Unify compatible render passes while differentiating execution passes:
attachmentIdxKeyInfo |= (!compatibleOnly && !rpData.fHasInputAttachment
        ? kAttachmentUnused
        : VulkanRenderPass::kColorAttachmentIdx) << 24;
```
