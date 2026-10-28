---
name: skia/tests/factor-precompile-shader-combination-helpers
description: When generating precompile test combinations or pipeline permutations, extract nested loop bodies and shader assembly blocks into dedicated helper functions
rubric: design
severity: should_fix
---

**Rule.** When generating precompile test combinations or pipeline permutations,
factor multi-step PrecompileShader graph assembly and sequential scoped blocks
into dedicated helper functions. In particular, extract inner loop bodies into
named helpers rather than inlining deeply nested combination blocks that cause
excessive indentation or duplicate shader wrapping across paint options.

**Why.** Inlining multi-step shader construction and multiple scoped blocks
inside nested permutation loops causes excessive indentation depth and obscures
test structure. Factoring each distinct scenario or combination into a
well-named helper function keeps nesting flat, clarifies intent, and avoids
duplicating shader wrapping logic across render pass configurations.

## Examples

Bad:

```cpp
void generate_combinations(...) {
    for (const auto& linearEffect : linearEffects) {
        // Deeply nested scoped blocks duplicate shader wrapping logic
        {
            sk_sp<PrecompileShader> wrapped = wrap_in_tonemap(effectManager, hdrImg);
            sk_sp<PrecompileShader> linear = wrap_in_linear_effect(effectManager, std::move(wrapped));
            PaintOptions paintOptions;
            paintOptions.setShaders({{std::move(linear)}});
            result.push_back({paintOptions, DrawTypeFlags::kNonAAFillRect, kRGBA_1_D_SRGB});
        }
        {
            sk_sp<PrecompileShader> wrapped = wrap_in_tonemap(effectManager, hdrImg);
            sk_sp<PrecompileShader> linear = wrap_in_linear_effect(effectManager, std::move(wrapped));
            PaintOptions paintOptions;
            paintOptions.setShaders({{std::move(linear)}});
            result.push_back({paintOptions, DrawTypeFlags::kNonAAFillRect, kRGBA16F_1_D_SRGB});
        }
    }
}
```

Good:

```cpp
void linear_tonemap_hdr(RuntimeEffectManager& effectManager,
                        sk_sp<PrecompileShader> hdrImg,
                        std::vector<PrecompileSettings>& result) {
    sk_sp<PrecompileShader> wrapped = wrap_in_tonemap(effectManager, hdrImg);
    sk_sp<PrecompileShader> linear = wrap_in_linear_effect(effectManager, std::move(wrapped));

    PaintOptions paintOptions;
    paintOptions.setShaders({{std::move(linear)}});
    result.push_back({paintOptions, DrawTypeFlags::kNonAAFillRect, kRGBA16F_1_D_SRGB});
    result.push_back({paintOptions, DrawTypeFlags::kNonAAFillRect, kRGBA_1_D_SRGB});
}

void generate_combinations(...) {
    for (const auto& linearEffect : linearEffects) {
        linear_tonemap_hdr(effectManager, hdrImg, result);
    }
}
```
