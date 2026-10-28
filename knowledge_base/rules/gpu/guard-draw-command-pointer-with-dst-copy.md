---
name: skia/gpu/guard-draw-command-pointer-with-dst-copy
description: When recording texture and sampler bindings with dst-copy textures, guard draw command pointers and index command resource arrays using command-specific counts rather than combined totals
rubric: correctness
severity: should_fix
---

**Rule.** When recording texture and sampler descriptors or bindings where
pipeline-level synthesized textures (such as dst-copy textures) are combined
with draw commands, do not assume the draw command pointer is non-null merely
because total texture counts exceed zero; guard command pointer dereferences
with an explicit `if (command)`. Additionally, guard access to command-specific
resource arrays (such as `command.fTextureIndices`) using the command's own
count (`command.fNumTexSamplers`) rather than the combined texture count, as
commands with zero textures can still have a positive combined count when
dst-copy textures are present.

**Why.** Intrinsic pipeline resources like dst-copy textures are synthesized by
the command buffer and increment combined texture counts, but they do not
populate draw command structures or guarantee a non-null draw command.
Dereferencing a null command pointer or indexing into `command.fTextureIndices`
based on a positive combined count causes null pointer dereferences,
out-of-bounds reads, or misidentifying non-existent command textures as static
samplers.

## Examples

Bad:

```cpp
// Variant 1: Dereferencing command without null check when numTexSamplers includes dstCopy
TArray<DescriptorData> descriptors(numTexSamplers);
for (int i = 0; i < command->fNumTexSamplers; i++) {
    const Sampler* immutableSampler = fActiveGraphicsPipeline->immutableSampler(i);
    descriptors.push_back({DescriptorType::kCombinedTextureSampler, 1, i,
                           PipelineStageFlags::kFragmentShader, immutableSampler});
}

// Variant 2: Indexing command resource array based on combined texture count
if (numTexturesAndSamplers) {
    const wgpu::YCbCrVkDescriptor& ycbcrDesc =
            TextureInfos::GetDawnTextureSpec(
                    drawPass.getTexture(command.fTextureIndices[0])->textureInfo())
                    .fYcbcrVkDescriptor;
    usingSingleStaticSampler = ycbcrUtils::DawnDescriptorIsValid(ycbcrDesc);
}
```

Good:

```cpp
// Variant 1: Guard command dereference before iterating command samplers
TArray<DescriptorData> descriptors(numTexSamplers);
if (command) {
    for (int i = 0; i < command->fNumTexSamplers; i++) {
        const Sampler* immutableSampler = fActiveGraphicsPipeline->immutableSampler(i);
        descriptors.push_back({DescriptorType::kCombinedTextureSampler, 1, i,
                               PipelineStageFlags::kFragmentShader, immutableSampler});
    }
}

// Variant 2: Guard command resource indexing using command-specific count
if (command.fNumTexSamplers == 1) {
    const wgpu::YCbCrVkDescriptor& ycbcrDesc =
            TextureInfos::GetDawnTextureSpec(
                    drawPass.getTexture(command.fTextureIndices[0])->textureInfo())
                    .fYcbcrVkDescriptor;
    usingSingleStaticSampler = ycbcrUtils::DawnDescriptorIsValid(ycbcrDesc);
}
```
