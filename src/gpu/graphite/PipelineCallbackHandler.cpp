/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/PipelineCallbackHandler.h"
#include "include/gpu/graphite/PrecompileContext.h"

namespace skiatools::graphite {

PipelineCallBackHandler::PipelineCallBackHandler() {}

PipelineCallBackHandler::~PipelineCallBackHandler() {
    if (fFile) {
        fclose(fFile);
    }
}

void PipelineCallBackHandler::add(sk_sp<SkData> payload) {
    if (!fFile) {
        fFile = fopen("/Users/robertphillips/chrome-log.txt", "a");
        if (!fFile) {
            printf("----------------------- Failed to open output file\n");
        }
    }

    SkAutoSpinlock lock{ fSpinLock };

    const sk_sp<SkData>* data = fMap.find({ payload.get() });
    if (!data) {
        if (fFile && fPCC) {
            std::string label = fPCC->getPipelineLabel(payload);
            printf("%s\n", label.c_str());
            fprintf(fFile, "--------------\n");
            fprintf(fFile, "%s\n", label.c_str());
            fflush(fFile);
        }

        fMap.set(std::move(payload));
    } else {
        printf("----------------------- Failed to log\n");
    }
}

void PipelineCallBackHandler::retrieve(
        std::vector<sk_sp<SkData>>* result) {
    SkAutoSpinlock lock{ fSpinLock };

    result->reserve(fMap.count());

    fMap.foreach([result](sk_sp<SkData>* data) {
        result->push_back(*data);
    });
}

void PipelineCallBackHandler::reset() {
    SkAutoSpinlock lock{ fSpinLock };

    fMap.reset();
}

}  // namespace skiatools::graphite
