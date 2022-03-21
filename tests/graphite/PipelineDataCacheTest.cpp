/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/Recorder.h"
#include "experimental/graphite/src/PipelineDataCache.h"
#include "experimental/graphite/src/RecorderPriv.h"
#include "src/core/SkPipelineData.h"
#include "src/core/SkUniform.h"

using namespace skgpu;

namespace {

std::unique_ptr<SkPipelineData::UniformDataBlock> make_udb(int numUniforms, int dataSize) {
    static constexpr int kMaxUniforms = 3;
    static constexpr SkUniform kUniforms[kMaxUniforms] {
        {"point0",   SkSLType::kFloat2 },
        {"point1",   SkSLType::kFloat2 },
        {"point2",   SkSLType::kFloat2 },
    };

    SkASSERT(numUniforms <= kMaxUniforms);

    sk_sp<SkUniformData> ud = SkUniformData::Make(SkSpan<const SkUniform>(kUniforms, numUniforms),
                                                  dataSize);
    for (int i = 0; i < numUniforms; ++i) {
        ud->offsets()[i] = i;
    }
    for (int i = 0; i < dataSize; ++i) {
        ud->data()[i] = i % 255;
    }

    return std::make_unique<SkPipelineData::UniformDataBlock>(std::move(ud));
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(PipelineDataCacheTest, reporter, context) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    auto cache = recorder->priv().uniformDataCache();

    REPORTER_ASSERT(reporter, cache->count() == 0);

    // Nullptr should already be in the cache
    {
        UniformDataCache::Index invalid;
        REPORTER_ASSERT(reporter, !invalid.isValid());

        SkPipelineData::UniformDataBlock* lookup = cache->lookup(invalid);
        REPORTER_ASSERT(reporter, !lookup);
    }

    // Add a new unique UDB
    SkPipelineData::UniformDataBlock* danglingUDB1 = nullptr;
    UniformDataCache::Index result1;
    {
        std::unique_ptr<SkPipelineData::UniformDataBlock> udb1 = make_udb(2, 16);
        danglingUDB1 = udb1.get();
        result1 = cache->insert(*udb1);
        REPORTER_ASSERT(reporter, result1.isValid());
        SkPipelineData::UniformDataBlock* lookup = cache->lookup(result1);
        REPORTER_ASSERT(reporter, lookup == danglingUDB1);

        REPORTER_ASSERT(reporter, cache->count() == 1);
    }

    // Try to add a duplicate PD
    {
        std::unique_ptr<SkPipelineData::UniformDataBlock> udb2 = make_udb(2, 16);
        SkPipelineData::UniformDataBlock* danglingUDB2 = udb2.get();
        UniformDataCache::Index result2 = cache->insert(*udb2);
        REPORTER_ASSERT(reporter, result2.isValid());
        REPORTER_ASSERT(reporter, result2 == result1);
        SkPipelineData::UniformDataBlock* lookup = cache->lookup(result2);
        REPORTER_ASSERT(reporter, lookup != danglingUDB2);
        REPORTER_ASSERT(reporter, lookup == danglingUDB1);

        REPORTER_ASSERT(reporter, cache->count() == 1);
    }

    // Add a second new unique PD
    {
        std::unique_ptr<SkPipelineData::UniformDataBlock> udb3 = make_udb(3, 16);
        SkPipelineData::UniformDataBlock* danglingUDB3 = udb3.get();
        UniformDataCache::Index result3 = cache->insert(*udb3);
        REPORTER_ASSERT(reporter, result3.isValid());
        REPORTER_ASSERT(reporter, result3 != result1);
        SkPipelineData::UniformDataBlock* lookup = cache->lookup(result3);
        REPORTER_ASSERT(reporter, lookup == danglingUDB3);
        REPORTER_ASSERT(reporter, lookup != danglingUDB1);

        REPORTER_ASSERT(reporter, cache->count() == 2);
    }

    // TODO(robertphillips): expand this test to exercise all the PD comparison failure modes
}
