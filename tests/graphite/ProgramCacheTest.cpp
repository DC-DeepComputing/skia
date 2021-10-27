/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/src/ContextPriv.h"
#include "experimental/graphite/src/ProgramCache.h"

using namespace skgpu;

DEF_GRAPHITE_TEST_FOR_CONTEXTS(ProgramCacheTest, reporter, context) {
    ProgramCache* cache = context->priv().programCache();

    REPORTER_ASSERT(reporter, cache->count() == 0);

    // Add an initial unique program
    Combination c1 {  ShaderCombo::ShaderType::kNone, SkTileMode::kRepeat, SkBlendMode::kSrc };
    uint32_t id1 = cache->findOrCreateProgram(c1);
    REPORTER_ASSERT(reporter, id1 != ProgramCache::kInvalidProgramID);

    REPORTER_ASSERT(reporter, cache->count() == 1);

    // Try to add a duplicate program
    uint32_t id2 = cache->findOrCreateProgram(c1);
    REPORTER_ASSERT(reporter, id2 != ProgramCache::kInvalidProgramID);
    REPORTER_ASSERT(reporter, id2 == id1);

    REPORTER_ASSERT(reporter, cache->count() == 1);

    // Add a second unique program
    Combination c2 {  ShaderCombo::ShaderType::kLinearGradient,
                      SkTileMode::kRepeat,
                      SkBlendMode::kSrc };
    uint32_t id3 = cache->findOrCreateProgram(c2);
    REPORTER_ASSERT(reporter, id3 != ProgramCache::kInvalidProgramID);
    REPORTER_ASSERT(reporter, id3 != id1);

    REPORTER_ASSERT(reporter, cache->count() == 2);

    // TODO(robertphillips): expand this test to exercise more program variations

}
