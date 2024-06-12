/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileShader_DEFINED
#define skgpu_graphite_precompile_PrecompileShader_DEFINED

#include "include/gpu/graphite/precompile/PrecompileBase.h"

#include "include/core/SkBlendMode.h"

class SkColorSpace;

namespace skgpu::graphite {

class PrecompileBlender;
class PrecompileColorFilter;

class SK_API PrecompileShader : public PrecompileBase {
public:
    sk_sp<PrecompileShader> makeWithLocalMatrix();

    sk_sp<PrecompileShader> makeWithColorFilter(sk_sp<PrecompileColorFilter>);

    sk_sp<PrecompileShader> makeWithWorkingColorSpace(sk_sp<SkColorSpace>);

protected:
    friend class PaintOption; // for isConstant access

    PrecompileShader() : PrecompileBase(Type::kShader) {}

//!!!!!!!!!!!
public:
    virtual bool isConstant(int desiredCombination) const { return false; }
};

//--------------------------------------------------------------------------------------------------
namespace PrecompileShaders {
    // --- This block of six matches the SkShaders factories in include/core/SkShader.h
    SK_API sk_sp<PrecompileShader> Empty();
    SK_API sk_sp<PrecompileShader> Color();
    SK_API sk_sp<PrecompileShader> Color(sk_sp<SkColorSpace>);
    SK_API sk_sp<PrecompileShader> Blend(SkSpan<SkBlendMode> blendModes,
                                         SkSpan<const sk_sp<PrecompileShader>> dsts,
                                         SkSpan<const sk_sp<PrecompileShader>> srcs);
    SK_API sk_sp<PrecompileShader> Blend(SkSpan<const sk_sp<PrecompileBlender>> blenders,
                                         SkSpan<const sk_sp<PrecompileShader>> dsts,
                                         SkSpan<const sk_sp<PrecompileShader>> srcs);
    SK_API sk_sp<PrecompileShader> CoordClamp(SkSpan<const sk_sp<PrecompileShader>>);

    // --- This block of two matches the SkShaders factories in include/effects/SkPerlinNoiseShader.h
    SK_API sk_sp<PrecompileShader> MakeFractalNoise();
    SK_API sk_sp<PrecompileShader> MakeTurbulence();

    // --- This block of two matches the SkShaders factories in include/core/SkImage.h
    // In the normal Skia API ImageShaders are usually created via a SkImage::makeShader call.
    // Since the SkImage used to create the ImageShader is unlikely to be present at precompilation
    // time this entry point allows the equivalent precompilation program structure to be created.
    SK_API sk_sp<PrecompileShader> Image();
    // As with the above Image call, raw ImageShaders are usually created via an
    // SkImage::makeRawShader call. The RawImage call allows the equivalent precompilation
    // program structure to be created without needing the SkImage.
    SK_API sk_sp<PrecompileShader> RawImage();

    // ??
    SK_API sk_sp<PrecompileShader> YUVImage();

    // TODO: make SkGradientShader match this convention (skbug.com/13438)
    // This block of four matches all the entry points in include/effects/SkGradientShader.h
    SK_API sk_sp<PrecompileShader> LinearGradient();
    SK_API sk_sp<PrecompileShader> RadialGradient();
    SK_API sk_sp<PrecompileShader> TwoPointConicalGradient();
    SK_API sk_sp<PrecompileShader> SweepGradient();

    // Normally, SkPicture shaders are only created via SkPicture::makeShader. Since the
    // SkPicture to be drawn, most likely, won't be available at precompilation time, this
    // entry point can be used to create a precompilation equivalent.
    // Note: this will precompile the program that draws the SkPicture. It, obviously, won't
    // precompile any SkPaints within the SkPicture.
    //
    // API Note: At the end of the day this turns into a LMShader wrapping an image shader. The
    // LMShader has logic to elide itself if the LM is missing or the Identity. Combinatorially,
    // this yields 6 combinations: 2 from the LM x 3 from the ImageShader. We could try to reduce
    // that by adding a "passing-non-null-non-Identity-LM-to-SkPicture::makeShader" flag here
    // in which case we would either add or skip the LMShader. That would be a pretty obscure API
    // though.
    SK_API sk_sp<PrecompileShader> Picture();

    // Normally, LocalMatrixShaders are only created via SkShader::makeWithLocalMatrix.
    // However, in the combination API, clients may want to create a set of precompile
    // LocalMatrixShaders (i.e., pass an SkSpan to the factory function vs just creating a
    // single option). This entry point allows that use case.
    // Note: PrecompileShader::makeWithLocalMatrix() can still be used and works as expected.
    SK_API sk_sp<PrecompileShader> LocalMatrix(SkSpan<const sk_sp<PrecompileShader>> wrapped);

    // Normally, ColorFilterShaders are only created via SkShader::makeWithColorFilter.
    // However, in the combination API, clients may want to create a set of precompile
    // ColorFilterShaders (i.e., pass SkSpans to the factory function vs just creating a
    // single option). This entry point allows that use case.
    // Note: PrecompileShader::makeWithColorFilter can still be used and works as expected.
    SK_API sk_sp<PrecompileShader> ColorFilter(
            SkSpan<const sk_sp<PrecompileShader>> shaders,
            SkSpan<const sk_sp<PrecompileColorFilter>> colorFilters);

    // Normally, WorkingColorSpaceShaders are only created via SkShader::makeWithWorkingColorSpace.
    // However, in the combination API, clients may want to create a set of precompile
    // WorkingColorSpaceShaders (i.e., pass SkSpans to the factory function vs just creating a
    // single option). This entry point allows that use case.
    // Note: PrecompileShader::makeWithWorkingColorSpace can still be used and works as expected.
    SK_API sk_sp<PrecompileShader> WorkingColorSpace(SkSpan<const sk_sp<PrecompileShader>> shaders,
                                                     SkSpan<const sk_sp<SkColorSpace>> colorSpaces);

} // namespace PrecompileShaders

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileShader_DEFINED
