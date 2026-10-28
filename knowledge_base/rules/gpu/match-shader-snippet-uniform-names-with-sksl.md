---
name: skia/gpu/match-shader-snippet-uniform-names-with-sksl
description: When declaring shader snippet uniforms in Graphite ShaderCodeDictionary, match uniform names with SkSL parameter names and packed vector component order
rubric: readability
severity: should_fix
---

**Rule.** When declaring snippet uniforms in ShaderCodeDictionary, align uniform
names with the parameter names of the corresponding static SkSL snippet function
and ensure names accurately reflect packed vector component ordering (e.g.,
'srcGABC' and 'dstGABC' for G, A, B, C parameters rather than 'srcABCG'). Avoid
divergent names between C++ dictionary definitions and SkSL signatures.

**Why.** Snippet uniform declarations in ShaderCodeDictionary provide the names
used in generated shader code when invoking static SkSL functions. Mismatches
between uniform identifiers, SkSL parameter names, and packed component order
obscure argument mapping, impair shader readability, and complicate debugging
during shader inspection.

## Examples

Bad:

```cpp
// In ShaderCodeDictionary.cpp: uniform names diverge from SkSL function parameters and component order:
fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kColorSpaceXformSRGB] = {
    /*name=*/"ColorSpaceTransformSRGB",
    /*staticFn=*/"sk_color_space_transform_srgb",
    SnippetRequirementFlags::kPriorStageOutput,
    /*uniforms=*/{ { "gamut",       SkSLType::kHalf3x3 },
                   { "srcABCG",     SkSLType::kHalf4 },
                   { "srcDEF_args", SkSLType::kHalf4 },
                   { "dstABCG",     SkSLType::kHalf4 },
                   { "dstDEF_args", SkSLType::kHalf4 } }
};
```

Good:

```cpp
// Uniform names match SkSL parameter names and packed component order (G, A, B, C):
fBuiltInCodeSnippets[(int) BuiltInCodeSnippetID::kColorSpaceXformSRGB] = {
    /*name=*/"ColorSpaceTransformSRGB",
    /*staticFn=*/"sk_color_space_transform_srgb",
    SnippetRequirementFlags::kPriorStageOutput,
    /*uniforms=*/{ { "gamut",       SkSLType::kHalf3x3 },
                   { "srcGABC",     SkSLType::kHalf4 },
                   { "srcDEF_args", SkSLType::kHalf4 },
                   { "dstGABC",     SkSLType::kHalf4 },
                   { "dstDEF_args", SkSLType::kHalf4 } }
};
```
