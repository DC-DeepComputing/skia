/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/text/gpu/TextBlobTools.h"

#include "src/text/gpu/SubRunContainer.h"
#include "src/text/gpu/TextBlob.h"
#include <cstdio>

namespace sktext::gpu {

const AtlasSubRun* TextBlobTools::FirstSubRun(const SubRunContainer* container) {
    SkASSERT(container);
    if (container->fSubRuns.isEmpty()) {
        return nullptr;
    }
    return container->fSubRuns.front().testingOnly_atlasSubRun();
}

const AtlasSubRun* TextBlobTools::FirstSubRun(const TextBlob* blob) {
    SkASSERT(blob);
    return FirstSubRun(blob->fSubRuns.get());
}

void TextBlobTools::PrintSubRuns(const SubRunContainer* container) {
    SkASSERT(container);
    fprintf(stderr, "SubRunContainer fSubRuns isEmpty: %d\n", container->fSubRuns.isEmpty());
    int i = 0;
    for (const SubRun& subRun : container->fSubRuns) {
        const AtlasSubRun* atlasSubRun = subRun.testingOnly_atlasSubRun();
        fprintf(stderr, "  SubRun[%d]: tag: %d, isAtlasSubRun: %d\n",
                i++, (int)subRun.subRunStreamTag(), atlasSubRun != nullptr);
    }
}

}  // namespace sktext::gpu
