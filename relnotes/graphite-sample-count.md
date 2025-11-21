Define a new public enum, `SampleCount`, that enforces the valid sample count values that Graphite
supports (1, 2, 4, 8, 16). `TextureInfo::numSamples() -> uint8_t` is replaced with
`TextureInfo::sampleCount() -> SampleCount`.

Backend specific texture infos, e.g. `DawnTextureInfo`,
`VulkanTextureInfo`, and `MtlTextureInfo`'s `fSampleCount` field, and the
`ContextOptions::fInternalMultisampleCount` field are now `SampleCount`. A helper
function, `ToSampleCount(uint32_t) -> SampleCount` is provided if needing to convert a variable value vs. just updating a constant.
