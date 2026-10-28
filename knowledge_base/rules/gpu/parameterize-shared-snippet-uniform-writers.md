---
name: skia/gpu/parameterize-shared-snippet-uniform-writers
description: When serializing uniforms for Graphite shader snippets, reuse shared packing helpers across matching layouts and parameterize them if snippet expectations differ
rubric: design
severity: should_fix
---

**Rule.** When multiple shader snippets share identical or overlapping uniform
requirements, extract and reuse a shared serialization helper rather than
duplicating uniform-packing code across snippet functions. When a shared helper
writes uniforms for snippet variants that differ in layout, parameterize or
branch the helper (such as via boolean flags) so emitted uniforms strictly match
the declared uniform expectations of each snippet caller.

**Why.** Duplicating uniform packing across snippet functions causes code bloat
and maintenance divergence when uniform layouts change. Conversely,
unconditionally emitting uniforms for all snippets from a shared helper violates
snippet-specific uniform declarations, causing buffer misalignment and
validation failures in UniformExpectationsValidator.

## Examples

Bad:

```cpp
// 1. Duplicating uniform serialization across snippet variants:
void add_srgb_uniform_data(..., PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kColorSpaceXformSRGB)
    // 50 lines duplicating gamut transform and transfer function serialization...
}

// 2. Unconditionally writing uniforms declared only for specific snippets:
void add_color_space_uniforms(..., PipelineDataGatherer* gatherer) {
    gatherer->write(SkV4{0.f, 0.f, 0.f, 0.f}); // Fails on snippets without ootf uniforms
}
```

Good:

```cpp
// 1. Reusing shared helper across snippets:
void add_srgb_uniform_data(..., PipelineDataGatherer* gatherer) {
    BEGIN_WRITE_UNIFORMS(gatherer, dict, BuiltInCodeSnippetID::kColorSpaceXformSRGB)
    add_color_space_uniforms(data.fSteps, /*has_ootf=*/false, gatherer);
}

// 2. Parameterizing helper to match each snippet's declared layout:
void add_color_space_uniforms(..., bool has_ootf, PipelineDataGatherer* gatherer) {
    if (has_ootf) {
        gatherer->write(SkV4{0.f, 0.f, 0.f, 0.f});
    }
}
```
