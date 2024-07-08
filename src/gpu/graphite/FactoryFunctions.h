/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_FactoryFunctions_DEFINED
#define skgpu_graphite_FactoryFunctions_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/effects/SkRuntimeEffect.h"

namespace skgpu::graphite {

class PrecompileBase;
class PrecompileBlender;
class PrecompileColorFilter;
class PrecompileImageFilter;
class PrecompileMaskFilter;
class PrecompileShader;

namespace PrecompileShaders {
    // ??
    SK_API sk_sp<PrecompileShader> YUVImage();

} // namespace PrecompileShaders

#if 0
//--------------------------------------------------------------------------------------------------
// Object that allows passing a SkPrecompileShader, SkPrecompileColorFilter or
// SkPrecompileBlender as a child
//
// This will moved to be on SkRuntimeEffect
class PrecompileChildPtr {
public:
    PrecompileChildPtr() = default;
    PrecompileChildPtr(sk_sp<PrecompileShader>);
    PrecompileChildPtr(sk_sp<PrecompileColorFilter>);
    PrecompileChildPtr(sk_sp<PrecompileBlender>);

    // Asserts that the SkPrecompileBase is either null, or one of the legal derived types
    PrecompileChildPtr(sk_sp<PrecompileBase>);

    std::optional<SkRuntimeEffect::ChildType> type() const;

    PrecompileShader* shader1() const;
    PrecompileColorFilter* colorFilter1() const;
    PrecompileBlender* blender1() const;
    PrecompileBase* base1() const { return fChild.get(); }

private:
    sk_sp<PrecompileBase> fChild;
};
#endif

using PrecompileChildOptions = SkSpan<const sk_sp<PrecompileBase>>;

// These will move to be on SkRuntimeEffect to parallel makeShader, makeColorFilter and
// makeBlender
sk_sp<PrecompileShader> MakePrecompileShader(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions = {});

sk_sp<PrecompileColorFilter> MakePrecompileColorFilter(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions = {});

sk_sp<PrecompileBlender> MakePrecompileBlender(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions = {});

} // namespace skgpu::graphite

#endif // skgpu_graphite_FactoryFunctions_DEFINED
