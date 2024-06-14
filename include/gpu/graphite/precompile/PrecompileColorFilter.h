/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileColorFilter_DEFINED
#define skgpu_graphite_precompile_PrecompileColorFilter_DEFINED

#include "include/gpu/graphite/precompile/PrecompileBase.h"

namespace skgpu::graphite {


class SK_API PrecompileColorFilter : public PrecompileBase {
public:
    sk_sp<PrecompileColorFilter> makeComposed(sk_sp<PrecompileColorFilter> inner) const;

protected:
    PrecompileColorFilter() : PrecompileBase(Type::kColorFilter) {}
    ~PrecompileColorFilter() override;
};

//--------------------------------------------------------------------------------------------------
// This is the Precompile correlate to the XXXX namespace in the main API
namespace PrecompileColorFilters {

} // namespace PrecompileColorFilters

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileColorFilter_DEFINED
