---
name: skia/remove-obsolete-assertions
description: When refactoring code or relaxing invariants, delete obsolete SkASSERT checks instead of contorting them, but preserve non-redundant precondition assertions unless callees explicitly verify them
rubric: correctness
severity: should_fix
---

**Rule.** When refactoring code or relaxing architectural invariants, remove
obsolete assertions (such as SkASSERT) entirely rather than adapting or
contorting them into artificial conditions that mask relaxed invariants.
Conversely, when refactoring methods to delegate to new or shared helpers, do
not remove existing precondition or invariant assertions under the assumption
that the callee handles them unless the helper or downstream calls explicitly
validate the exact same condition.

**Why.** Assertions exist to document and enforce true program invariants and
developer expectations. When architectural assumptions are relaxed, contorting
obsolete assertions creates confusing, dead, or false-positive checks;
conversely, dropping still-valid precondition assertions when delegating to
helpers eliminates valuable debug-time verification and allows invalid caller
states to proceed undetected.

## Examples

Bad:

```cpp
// Bad: Contorting an obsolete mutual-exclusion check after generalizing push constants:
SkASSERT(pushConstantSize > 0 ||
         (!hasStepUniforms && !hasPaintUniforms && !hasGradientBuffer) ||
         sharedContext->vulkanCaps().storageBufferSupport());

// Bad: Removing a valid precondition assertion assuming a downstream helper checks it:
FilterResult FilterResult::insetForSaveLayer() const {
    if (!fImage) {
        return {};
    }
    // Mistakenly removed SkASSERT(fTileMode == SkTileMode::kDecal) assuming insetByPixel() checked it
    FilterResult inset = this->insetByPixel();
    ...
```

Good:

```cpp
// Good: Deleting the assertion entirely because push constants and uniform buffers can now coexist:
static VkPipelineLayout setup_pipeline_layout(const VulkanSharedContext* sharedContext,
                                              uint32_t pushConstantSize,
                                              VkShaderStageFlagBits pushConstantPipelineStageFlags, ...) {
    // Proceed to configure descriptor sets and layout without invalid invariant checks
}

// Good: Preserving the precondition assertion with documentation:
FilterResult FilterResult::insetForSaveLayer() const {
    if (!fImage) {
        return {};
    }
    // SkCanvas processing should have prepared a decal-tiled image before calling this.
    SkASSERT(fTileMode == SkTileMode::kDecal);
    FilterResult inset = this->insetByPixel();
    ...
```
