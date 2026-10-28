---
name: skia/gpu/sign-encode-flags-in-float-uniforms
description: When passing GPU shader modes or flags, pack them into unused vector components, sentinel values, degenerate states, or parameter signs instead of allocating dedicated uniforms
rubric: safety
severity: should_fix
---

**Rule.** When passing boolean flags or operational modes to GPU shaders or SkSL
runtime effects, avoid allocating separate integer or boolean uniforms when
states can be packed into unused components of existing float/half vector
uniforms (such as expanding half2 to half3), encoded into sentinel values or
degenerate states, or sign-encoded into non-negative float parameters (such as
insets, dimensions, or radii). Document the encoding convention where values are
unpacked or evaluated in the shader. In shader code, test sign-encoded flags by
comparing directly against zero (param < 0 or param > 0) rather than calling
sign(param) < 0, and restore original magnitudes with abs(param).

**Why.** Each uniform passed to a GPU shader or runtime effect increases uniform
buffer memory footprint, layout alignment padding, and per-draw upload
bandwidth; furthermore, SkSL runtime effects do not support boolean or narrow
integer uniforms. Packing mode flags into adjacent vector components, unused
signs, or degenerate parameter states conserves uniform slots, while direct zero
and sentinel comparisons avoid redundant ALU calls and keep shader control flow
idiomatic.

## Examples

Bad:

```cpp
// Host: Passing dedicated uniforms for boolean modes alongside existing parameters:
gatherer->write(imgData.fLinearFilterUVInset);
gatherer->write(SkTo<int>(imgData.fSampling.filter));
gatherer->write(isRadial ? 1 : 0);
gatherer->write(a);
gatherer->write(invA);

// Runtime effect: Standalone int uniform for a 1-bit boolean flag:
"uniform half2 gainAndBias;"
"uniform int convolveAlpha;"

// Shader: Unpacking dedicated flags or using sign() without documenting conventions:
float2 radius = float2(abs(radiusPlusHalf.x));
float2 uvCoords = sign(linearFilterUVInset.x) < 0 ? floor(coords) + 0.5 : coords;
if (isRadial == 1) { ... }
```

Good:

```cpp
// Host: Encode modes into signs, sentinel states, or vector channels:
SkPoint linearFilterUVInset = imgData.fLinearFilterUVInset;
if (imgData.fSampling.filter == SkFilterMode::kNearest) {
    linearFilterUVInset.fX = -linearFilterUVInset.fX;
}
gatherer->write(linearFilterUVInset);

float a = 0, invA = 1; // Radial mode encoded via degenerate sentinel values (a == 0 && invA == 1)
if (!isRadial) {
    a = 1 - dRadius * dRadius;
    invA = (std::abs(a) > SK_ScalarNearlyZero) ? 1.0f / (2.0f * a) : 0.0f;
}
gatherer->write(a);
gatherer->write(invA);

// Runtime effect: Pack scalar flag into an available vector channel:
"uniform half3 gainBiasAndConvolveAlpha;"

// Shader: Document conventions, compare directly against zero or sentinels, and restore values:
// Negative x indicates nearest-neighbor filtering; restore magnitude with abs()
float2 uvCoords = linearFilterUVInset.x < 0 ? floor(coords) + 0.5 : coords;
linearFilterUVInset = abs(linearFilterUVInset);

// Radial case encoded via a == 0 && invA == 1
if (a == 0 && invA == 1) { ... }
```
