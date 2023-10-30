/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceCache.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SharedContext.h"

namespace skgpu::graphite {

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(Protected_SmokeTest_Graphite, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    if (!context->supportsProtectedContent()) {
        // Protected content not supported
        return;
    }

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    ResourceCache* resourceCache = resourceProvider->resourceCache();
    const SharedContext* sharedContext = resourceProvider->sharedContext();

}

} // namespace skgpu::graphite
