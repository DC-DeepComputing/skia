/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/private/GraphiteTypesPriv.h"
#include "experimental/graphite/src/ContextPriv.h"
#include "experimental/graphite/src/ContextUtils.h"
#include "experimental/graphite/src/Uniform.h"
#include "experimental/graphite/src/UniformCache.h"

using namespace skgpu;

namespace {

sk_sp<UniformData> make_ud(int numUniforms, int dataSize) {
    static constexpr int kMaxUniforms = 3;
    static constexpr Uniform kUniforms[kMaxUniforms] {
        {"point0",   SLType::kFloat2 },
        {"point1",   SLType::kFloat2 },
        {"point2",   SLType::kFloat2 },
    };

    SkASSERT(numUniforms <= kMaxUniforms);

    sk_sp<UniformData> ud = UniformData::Make(numUniforms,
                                              kUniforms,
                                              dataSize);
    for (int i = 0; i < numUniforms; ++i) {
        ud->offsets()[i] = i;
    }
    for (int i = 0; i < dataSize; ++i) {
        ud->data()[i] = i % 255;
    }

    return ud;
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(UniformCacheTest, reporter, context) {
    UniformCache* cache = context->priv().uniformCache();

    REPORTER_ASSERT(reporter, cache->count() == 0);

    // Adding a new unique UD
    sk_sp<UniformData> ud1 = make_ud(2, 16);
    uint32_t id1 = cache->findOrCreate(ud1);
    SkASSERT(id1 != UniformData::kInvalidUniformID);
    SkASSERT(ud1->id() == id1);

    REPORTER_ASSERT(reporter, cache->count() == 1);

    // Adding a duplicate UD
    sk_sp<UniformData> ud2 = make_ud(2, 16);
    uint32_t id2 = cache->findOrCreate(ud2);
    SkASSERT(id2 != UniformData::kInvalidUniformID);
    SkASSERT(ud2->id() == UniformData::kInvalidUniformID);
    SkASSERT(id2 == id1);

    REPORTER_ASSERT(reporter, cache->count() == 1);

    // Add a second new unique UD
    sk_sp<UniformData> ud3 = make_ud(3, 16);
    uint32_t id3 = cache->findOrCreate(ud3);
    SkASSERT(id3 != UniformData::kInvalidUniformID);
    SkASSERT(ud3->id() == id3);
    SkASSERT(id3 != id1);

    REPORTER_ASSERT(reporter, cache->count() == 2);

    // TODO(robertphillips): expand this test to exercise all the UD comparison failure modes
}
