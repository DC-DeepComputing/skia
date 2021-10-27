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

namespace {

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(ProgramCacheTest, reporter, context) {
    ProgramCache* cache = context->priv().programCache();

    REPORTER_ASSERT(reporter, cache->count() == 0);
}
