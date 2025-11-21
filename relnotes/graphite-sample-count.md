<<<<<<< PATCH SET (12ea5cf8e9d239a068a3477eb77beedfd5876c4d [graphite] Fully use SampleCount in TextureInfo::Datas and C)
Define a new public enum, `SampleCount`, that enforces the valid sample count values that Graphite
supports (1, 2, 4, 8, 16). `TextureInfo::numSamples() -> uint8_t` is replaced with
`TextureInfo::sampleCount() -> SampleCount`.

Backend specific texture infos, e.g. `DawnTextureInfo`,
`VulkanTextureInfo`, and `MtlTextureInfo`'s `fSampleCount` field, and the
`ContextOptions::fInternalMultisampleCount` field are now `SampleCount`. A helper
function, `ToSampleCount(uint32_t) -> SampleCount` is provided if needing to convert a variable value vs. just updating a constant.
||||||| BASE      (c588bb60d5da5dfb2bef78f965fff748f9c8c7f0 [graphite] Stage SampleCount enum with uint assignment)
Define a new public enum, `SampleCount`, that enforces the valid sample count values that Graphite
supports (1, 2, 4, 8, 16). `TextureInfo::numSamples() -> uint8_t` is replaced with
`TextureInfo::sampleCount() -> SampleCount`.

Backend specific texture infos, e.g. `DawnTextureInfo`,
`VulkanTextureInfo`, and `MtlTextureInfo` still represent sample count as a `uint8_t` for
convience with the backend APIs. This `uint8_t` value is validated when wrapping the backend info
into a `TextureInfo`; if it's not a `SampleCount` value, then an empty `TextureInfo` is returned.
=======
>>>>>>> BASE      (cfa5063a9385bc418b20ecb5d61a9c8c82bf954a Merge 7 release notes into RELEASE_NOTES.md)
