/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/precompile/PrecompileImageFilter.h"

#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
sk_sp<PrecompileColorFilter> PrecompileImageFilter::asAColorFilter() const {
    sk_sp<PrecompileColorFilter> tmp = this->isColorFilterNode();
    if (!tmp) {
        return nullptr;
    }
    SkASSERT(this->countInputs() == 1);
    if (this->getInput(0)) {
        return nullptr;
    }
    // TODO: as in SkImageFilter::asAColorFilter, handle the special case of
    // affectsTransparentBlack. This is tricky for precompilation since we don't,
    // necessarily, have all the parameters of the ColorFilter in order to evaluate
    // filterColor4f(SkColors::kTransparent) - the normal API's implementation.
    return tmp;
}

} // namespace skgpu::graphite
