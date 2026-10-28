---
name: skia/gpu/omit-redundant-sample-count-guard-before-compatible-msaa
description: When querying compatible MSAA sample counts via Caps::getCompatibleMSAASampleCount, omit redundant sample count checks since it handles already-multisampled textures internally
rubric: readability
severity: nit
---

**Rule.** Do not guard calls to Caps::getCompatibleMSAASampleCount(textureInfo)
with checks testing whether the texture is single-sampled (such as
textureInfo.sampleCount() == SampleCount::k1). Invoke
getCompatibleMSAASampleCount directly, relying on its internal check to return
the existing sample count if the texture is already multisampled.

**Why.** Caps::getCompatibleMSAASampleCount internally checks if the target
texture already has a sample count greater than 1 and immediately returns it.
Adding an external single-sampled guard at the call site duplicates this check
and complicates conditional expressions.

## Examples

Bad:

```cpp
desc.fSampleCount = requiresMSAA && targetInfo.sampleCount() == SampleCount::k1
        ? caps->getCompatibleMSAASampleCount(targetInfo)
        : targetInfo.sampleCount();
```

Good:

```cpp
desc.fSampleCount = requiresMSAA ? caps->getCompatibleMSAASampleCount(targetInfo)
                                 : targetInfo.sampleCount();
```
