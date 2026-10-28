---
name: skia/gpu/assert-exclusive-alpha-flags-in-specialized-color-xforms
description: In specialized color space transform shader helpers handling isolated alpha transitions, assert that unpremul and premul flags are mutually exclusive using SkASSERT
rubric: correctness
severity: should_fix
---

**Rule.** In specialized color-space transform shader helpers and uniform
generators that only handle isolated alpha transitions (such as dedicated premul
or unpremul stages), assert using SkASSERT(!(flags.unpremul && flags.premul))
that unpremul and premul flags are mutually exclusive. Do not omit the assertion
or assume callers have already validated alpha flag combinations.

**Why.** In SkColorSpaceXformSteps, unpremul and premul cancel each other out
unless intermediate gamut transformations or non-linear transfer functions exist
between them. Specialized shader snippets designed solely for isolated alpha
transitions cannot perform intermediate steps, so asserting mutual exclusivity
prevents silent misencoding or accidental snippet binding for unsupported
transforms.

## Examples

Bad:

```cpp
void add_color_space_xform_premul_uniform_data(
        const ShaderCodeDictionary* dict,
        const ColorSpaceTransformBlock::ColorSpaceTransformData& data,
        PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kColorSpaceXformPremul)
    SkASSERT(data.fReadSwizzle == ReadSwizzle::kRGBA || data.fReadSwizzle == ReadSwizzle::kRGB1);
    // Bad: Missing assertion that unpremul and premul flags are mutually exclusive
}
```

Good:

```cpp
void add_color_space_xform_premul_uniform_data(
        const ShaderCodeDictionary* dict,
        const ColorSpaceTransformBlock::ColorSpaceTransformData& data,
        PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kColorSpaceXformPremul)
    SkASSERT(data.fReadSwizzle == ReadSwizzle::kRGBA || data.fReadSwizzle == ReadSwizzle::kRGB1);
    // Good: Enforce that specialized alpha step does not mix both unpremul and premul
    SkASSERT(!(data.fSteps.flags.unpremul && data.fSteps.flags.premul));
}
```
