---
name: skia/gpu/request-local-coords-for-clip-snippets
description: In Graphite clip shader snippets, specify SnippetRequirementFlags::kLocalCoords and accept float2 coords rather than reading sk_FragCoord directly to respect replay translation
rubric: correctness
severity: should_fix
---

**Rule.** In Graphite shader snippets that operate on device coordinates (such
as clip snippets) and their corresponding SkSL functions, specify
SnippetRequirementFlags::kLocalCoords in ShaderCodeDictionary and accept float2
coords as the first parameter rather than reading sk_FragCoord directly. Rely on
ShaderInfo to pass the replay-adjusted device coordinates (devCoord =
sk_FragCoord.xy - viewport.xy) as the coordinates argument.

**Why.** In Graphite, sk_FragCoord includes viewport replay translation offsets.
Clip shader snippets that read sk_FragCoord directly fail to account for this
offset when replay translation is active, causing clip evaluation and atlas
texture lookups to be displaced.

## Examples

Bad:

```cpp
// In ShaderCodeDictionary.cpp:
fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kAtlasClip] = {
        /*name=*/"AtlasClip",
        /*staticFn=*/"sk_atlas_clip",
        SnippetRequirementFlags::kNone,
        /*uniforms=*/{ ... }
};
// In sksl_graphite_frag.sksl:
$pure half4 sk_atlas_clip(half2 texCoordOffset, ...) {
    float2 texCoord = sk_FragCoord.xy + texCoordOffset;
    ...
}
```

Good:

```cpp
// In ShaderCodeDictionary.cpp:
fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kAtlasClip] = {
        /*name=*/"AtlasClip",
        /*staticFn=*/"sk_atlas_clip",
        SnippetRequirementFlags::kLocalCoords,
        /*uniforms=*/{ ... }
};
// In sksl_graphite_frag.sksl:
$pure half4 sk_atlas_clip(float2 coords, half2 texCoordOffset, ...) {
    float2 texCoord = coords + texCoordOffset;
    ...
}
```
