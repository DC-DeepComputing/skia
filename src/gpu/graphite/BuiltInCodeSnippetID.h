/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_BuiltInCodeSnippetID_DEFINED
#define skgpu_graphite_BuiltInCodeSnippetID_DEFINED

#include "include/core/SkTypes.h"

namespace skgpu::graphite {


// node function(half4 priorStageOutput, half4 dstColor, float2 pos)
//    -> extract child values
//    -> extract node uniform values
//    -> return staticFnc(..., )

// shader's node function (float2 pos)
// color filter function (half4 priorStageOutput)
// blend node function (half4 priorStageOutput("src"), half4 dstColor)

// switch blend mode node function (half4 src, half4 dst)
//   0 children
//   1 uniform for the blend mode enum
//   return sk_blend(enum, src, dst)

// coeff blend mode node function (half4 src, half4 dst)
//   0 children
//   1 uniform for the 4 blend coefficients
//   return porter_duff(coeff, src, dst)

enum class BuiltInCodeSnippetID : int32_t {
    // This isn't just a signal for a failure during paintparams key creation. It also actually
    // implements the default behavior for an erroneous draw. Currently it just draws solid
    // magenta.
    kError,

    // Magic reference blocks
    kDstColor,
    kPrimitiveColor,
    kPriorStageOutput,

    // SkShader code snippets
    // kPassthroughShader,
    kSolidColorShader,
    kLinearGradientShader4,
    kLinearGradientShader8,
    kLinearGradientShaderTexture,
    kRadialGradientShader4,
    kRadialGradientShader8,
    kRadialGradientShaderTexture,
    kSweepGradientShader4,
    kSweepGradientShader8,
    kSweepGradientShaderTexture,
    kConicalGradientShader4,
    kConicalGradientShader8,
    kConicalGradientShaderTexture,

    kLocalMatrixShader,
    kImageShader,
    kCoordClampShader,
    kDitherShader,
    kPerlinNoiseShader,
    // kPorterDuffBlendShader,     // ComposeShader (lightweight, only supports Porter-Duff blends)
    // kBlendShader,               // ComposeShader (more code, but supports every SkBlendMode)

    kTreeBlender, // 3 children, src, dst, and blender function
     push<treeBlender>, push<src shader root>, push <dst shader root>, push <blend func root>


    kColorFilterShader,
    kRuntimeShader,

    // SkColorFilter code snippets
    kMatrixColorFilter,
    // kBlendColorFilter,
       push<TreeBlender>, push<color filter>, push<solid colro block>, push<blend func>
    kComposeColorFilter,
    kTableColorFilter,
    kGaussianColorFilter,
    kColorSpaceXformColorFilter,

    // SkBlender code snippets (evaluating a blend as part of the shader graph)
    kSwitchBlender,
    kCoeffBlender,
    // kPassthroughBlender, // src-over


    // kPassthroughBlender,

    // BlendMode code snippets (applying a blend to a destination)
    kShaderBasedBlender,
       push<TreeBlender>, push<dither OR fColorFilter OR shader>, push<DstColor>, push<blend func>
    kPrimitiveColorShaderBasedBlender, // Blend dst: primitiveColor variable emitted by RenderStep
// Fixed-function blend modes are used for the final blend with the dst buffer's color when the
    // SkPaint is using a coefficient-based SkBlendMode. The actual coefficients are extracted into
    // the SkBlendInfo associated with each pipeline, but a unique code snippet ID is assigned so
    // that the pipeline keys remain distinct. They are ordered to match SkBlendMode such
    // that (id - kFirstFixedFunctionBlendMode) == SkBlendMode).
    //
    // NOTE: Pipeline code generation depends on the fixed-function code IDs being contiguous and
    // be defined last in the enum.
    kFixedFunctionClearBlendMode,
    kFixedFunctionSrcBlendMode,
    kFixedFunctionDstBlendMode,
    kFixedFunctionSrcOverBlendMode,
    kFixedFunctionDstOverBlendMode,
    kFixedFunctionSrcInBlendMode,
    kFixedFunctionDstInBlendMode,
    kFixedFunctionSrcOutBlendMode,
    kFixedFunctionDstOutBlendMode,
    kFixedFunctionSrcATopBlendMode,
    kFixedFunctionDstATopBlendMode,
    kFixedFunctionXorBlendMode,
    kFixedFunctionPlusBlendMode,
    kFixedFunctionModulateBlendMode,
    kFixedFunctionScreenBlendMode,

    kFirstFixedFunctionBlendMode = kFixedFunctionClearBlendMode,
    kLast = kFixedFunctionScreenBlendMode
};
static constexpr int kBuiltInCodeSnippetIDCount = static_cast<int>(BuiltInCodeSnippetID::kLast)+1;
static constexpr int kFixedFunctionBlendModeIDOffset =
        static_cast<int>(BuiltInCodeSnippetID::kFirstFixedFunctionBlendMode);

static_assert(BuiltInCodeSnippetID::kLast == BuiltInCodeSnippetID::kFixedFunctionScreenBlendMode);

} // skgpu::graphite

#endif // skgpu_graphite_BuiltInCodeSnippetID_DEFINED
