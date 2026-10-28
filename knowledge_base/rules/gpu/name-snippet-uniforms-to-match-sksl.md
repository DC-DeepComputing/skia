---
name: skia/gpu/name-snippet-uniforms-to-match-sksl
description: When defining Graphite shader snippet uniforms, name variables to match the corresponding SkSL module function parameters and accurately reflect packed component order
rubric: readability
severity: should_fix
---

**Rule.** When defining uniforms for Graphite shader snippets in
ShaderCodeDictionary, name each uniform variable to match the corresponding
parameter name in the snippet's SkSL module function (staticFn). For packed
vector uniforms, ensure the name accurately reflects the layout and order of
packed components (such as srcGABC for components {g, a, b, c}).

**Why.** Graphite's shader code generator pairs snippet uniform declarations
with static SkSL functions. Inconsistent uniform names obscure component layout,
make generated shader code harder to debug, and lead to subtle
component-ordering mistakes when binding uniforms.

## Examples

Bad:

```cpp
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
