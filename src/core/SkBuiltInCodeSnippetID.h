/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBuiltInCodeSnippetID_DEFINED
#define SkBuiltInCodeSnippetID_DEFINED

#include "include/core/SkTypes.h"

#define BUILT_IN_CODE_SNIPPETS(M) \
    M(Invalid)                    \
    M(DepthStencilOnlyDraw)       \
    M(Error)                      \
    M(SolidColorShader)           \
    M(LinearGradientShader)       \
    M(RadialGradientShader)       \
    M(SweepGradientShader)        \
    M(ConicalGradientShader)      \
    M(LocalMatrixShader)          \
    M(ImageShader)                \
    M(BlendShader)                \
    M(FixedFunctionBlender)       \
    M(ShaderBasedBlender)

#define ENUM(T) k##T,
enum class SkBuiltInCodeSnippetID2 : uint8_t { BUILT_IN_CODE_SNIPPETS(ENUM) };
static constexpr int kBuiltInCodeSnippetIDCount2 = static_cast<int>(SkBuiltInCodeSnippetID2::kShaderBasedBlender)+1;
#undef ENUM

#define FOO(T) constexpr SkSnippetID k##T##SnippetID((int) SkBuiltInCodeSnippetID2::k##T);
BUILT_IN_CODE_SNIPPETS(FOO)
#undef FOO

#if 0
enum class SkBuiltInCodeSnippetID : uint8_t {
    // TODO: It seems like this requires some refinement. Fundamentally this doesn't seem like a
    // draw that originated from a PaintParams.
    kDepthStencilOnlyDraw,

    // This isn't just a signal for a failure during paintparams key creation. It also actually
    // implements the default behavior for an erroneous draw. Currently it just draws solid
    // magenta.
    kError,

    // SkShader code snippets
    kSolidColorShader,
    kLinearGradientShader,
    kRadialGradientShader,
    kSweepGradientShader,
    kConicalGradientShader,

    kLocalMatrixShader,
    kImageShader,
    kBlendShader,     // aka ComposeShader

    // BlendMode code snippets
    kFixedFunctionBlender,
    kShaderBasedBlender,

    kLast = kShaderBasedBlender
};
static constexpr int kBuiltInCodeSnippetIDCount = static_cast<int>(SkBuiltInCodeSnippetID::kLast)+1;
#endif

#endif // SkBuiltInCodeSnippetID_DEFINED
