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

void PipelineCallBackHandler::add(sk_sp<SkData> payload, const std::string& label) {
    if (!fFile) {
        fFile = fopen("/Users/robertphillips/chrome-log.txt", "a");
        if (!fFile) {
            printf("----------------------- Failed to open output file\n");
        }
    }

    SkAutoSpinlock lock{ fSpinLock };

    Type7* data = fMap.find({ payload.get() });
    if (!data) {
        static int sID = 0;

        if (fFile && fPCC) {
//            std::string label = fPCC->getPipelineLabel(payload);
            fprintf(fFile, "%s\n", label.c_str());
            fflush(fFile);
        }

        fMap.set(Type7(sID++, std::move(payload)));
    } else {
//        fprintf(fFile, "%s\n", label.c_str());
//        fflush(fFile);
    }
}

void PipelineCallBackHandler::dumpAll() {
    if (!fFile) {
        fFile = fopen("/Users/robertphillips/chrome-log.txt", "a");
        if (!fFile) {
            printf("----------------------- Failed to open output file\n");
        }
    }

    SkAutoSpinlock lock{ fSpinLock };

    fprintf(fFile, "--------------\n");
    fprintf(fFile, "Shutting down\n");
    fflush(fFile);
}

void PipelineCallBackHandler::retrieve(
        std::vector<sk_sp<SkData>>* result) {
    SkAutoSpinlock lock{ fSpinLock };

    result->reserve(fMap.count());

    fMap.foreach([result](Type7* data) {
        result->push_back(data->fData);
    });
}

void PipelineCallBackHandler::reset() {
    SkAutoSpinlock lock{ fSpinLock };

    fMap.reset();
}

}  // namespace skiatools::graphite
