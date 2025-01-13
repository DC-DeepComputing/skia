/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_BlobOStuff_DEFINED
#define skgpu_graphite_precompile_BlobOStuff_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"
#include "src/gpu/ResourceKey.h"
//#include "include/core/SkSpan.h"

namespace skgpu::graphite {


class BlobOStuff {
public:
    BlobOStuff(int foo) : fFoo(foo) {}

    sk_sp<SkData> snap() const {
        return nullptr;
    }

private:
    [[maybe_unused]] int fFoo;
    skgpu::UniqueKey fPipelineKey;
};


} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_BlobOStuff_DEFINED
