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

class SkStream;

namespace skgpu::graphite {


class BlobOStuff {
public:
    static const int kCurrent_Version = 1;

    BlobOStuff(int foo) : fFoo(foo) {}

    static BlobOStuff* MakeFromStream(SkStream* stream);

    static BlobOStuff* MakeFromData(const SkData* data);

    static BlobOStuff* MakeFromData(const void* data, size_t size);

    sk_sp<SkData> snap() const {
        return nullptr;
    }

private:
    [[maybe_unused]] int fFoo;
    skgpu::UniqueKey fPipelineKey;
};


} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_BlobOStuff_DEFINED
