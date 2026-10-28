---
name: skia/gpu/initialize-external-format-sample-counts-in-caps
description: When querying texture format capabilities in backend Caps, explicitly initialize supported sample counts to single-sample and properties for TextureFormat::kExternal
rubric: correctness
severity: blocking
---

**Rule.** When querying texture format capabilities in GPU backend Caps (such as
getTextureSupport or initFormatTable), explicitly initialize supported sample
counts to SampleCount::k1 and format properties for TextureFormat::kExternal. Do
not leave intermediate pointers to backend format tables null or assume external
formats populate standard format table entries.

**Why.** TextureFormat::kExternal (used for platform-external textures like
Android Hardware Buffers) has no standard entry in backend format info tables
and maps to VK_FORMAT_UNDEFINED. Bypassing explicit capability initialization
leaves intermediate format pointers null, causing null pointer dereferences when
retrieving properties or sample counts. External textures do not support
multisampling, so sample counts must be explicitly set to single-sampled
(SampleCount::k1).

## Examples

Bad:

```cpp
if (format == TextureFormat::kExternal) {
    props = &kExternalProps;
    renderBits = 0;
    // supportedSampleCounts remains nullptr!
} else {
    supportedSampleCounts = &formatInfo.fSupportedSampleCounts;
}
...
sampleCounts = static_cast<SampleCount>(supportedSampleCounts->fSampleCounts);
```

Good:

```cpp
if (format == TextureFormat::kExternal) {
    props = &kExternalProps;
    renderBits = 0;
    sampleCounts = SampleCount::k1;
} else {
    sampleCounts = static_cast<SampleCount>(formatInfo.fSupportedSampleCounts.fSampleCounts);
}
```
