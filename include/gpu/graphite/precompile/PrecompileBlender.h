/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileBlender_DEFINED
#define skgpu_graphite_precompile_PrecompileBlender_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/gpu/graphite/precompile/PrecompileBase.h"

#include <optional>

namespace skgpu::graphite {

/** \class PrecompileBlender
    This class corresponds to the SkBlender class in the main API.
*/
class PrecompileBlender : public PrecompileBase {
public:
    virtual std::optional<SkBlendMode> asBlendMode() const { return {}; }

    /**
     * This is the Precompilation correlate to the SkBlender::Mode factory.
     */
    static sk_sp<PrecompileBlender> Mode(SkBlendMode);

protected:
    PrecompileBlender() : PrecompileBase(Type::kBlender) {}
    ~PrecompileBlender() override;
};

//--------------------------------------------------------------------------------------------------
// This is the Precompile correlate to the SkBlenders
namespace PrecompileBlenders {

    // This factory creates a Precompilation object that represents the Arithmetic SkBlender from
    // the main API
    SK_API sk_sp<PrecompileBlender> Arithmetic();

} // namespace PrecompileBlenders


} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileBlender_DEFINED
