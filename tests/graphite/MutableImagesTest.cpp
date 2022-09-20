/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#ifdef SK_GRAPHITE_ENABLED

#include "include/core/SkImage.h"

#include "include/gpu/graphite/Context.h"

using namespace skgpu::graphite;

namespace {

} // anonymous namespace


DEF_GRAPHITE_TEST_FOR_CONTEXTS(MutableImagesTest, reporter, context) {
    auto recorder = context->makeRecorder();

}

#endif // SK_GRAPHITE_ENABLED
