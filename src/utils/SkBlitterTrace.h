/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitterTrace_DEFINED
#define SkBlitterTrace_DEFINED

#include <inttypes.h>
#include <unordered_map>
#include "src/utils/SkCycles.h"

class SkBlitterTrace {
public:
    SkBlitterTrace(const char* header, bool traceSteps = false)
        : fHeader(header), fTraceSteps(traceSteps) { }

    SkBlitterTrace& operator= (const SkBlitterTrace&) = default;

    void addTrace(const char* name, uint64_t cycles, uint64_t scanLines, uint64_t pixels) {
        fCycles += cycles;
        fScanlines += scanLines;
        fPixels += pixels;
        if (fTraceSteps) {
            printIncrements(name, cycles, scanLines, pixels);
        }
    }

    void reset() {
        fCycles = 0ul;
        fScanlines = 0ul;
        fPixels = 0ul;
    }

    void printIncrements(const char* name, uint64_t cycles, uint64_t scanLines, uint64_t pixels) const {
        SkDebugf("%s %s: cycles=%" PRIu64 "+%" PRIu64
                       " scanlines=%" PRIu64 "+%" PRIu64 " pixels=%" PRIu64,
                 fHeader, name,
                 fCycles - cycles, cycles,
                 fScanlines - scanLines, scanLines,
                 fPixels);
        SkDebugf("\n");
    }

    void printCounts(const char* name) const {
        SkDebugf("%s cycles: %" PRIu64 " scanlines: %" PRIu64 " pixels: %" PRIu64,
                 fHeader,
                 fCycles,
                 fScanlines,
                 fPixels);
        SkDebugf(" ");
    }

    void turnTrace(bool value) { fTraceTime = value; }
    uint64_t getCycles() const { return fCycles; }
    uint64_t getScanlines() const { return fScanlines; }
    uint64_t getPixels() const { return fPixels; }

    class Step {
    public:
        Step(SkBlitterTrace* trace, const char* name, uint64_t scanlines, uint64_t pixels)
            : fTrace(trace)
            , fName(name)
            , fScanlines(scanlines)
            , fPixels(pixels) {
            fStartTime = SkCycles::Now();
        }
        void add(uint64_t scanlines, uint64_t pixels) {
            fScanlines += scanlines;
            fPixels += pixels;
        }
        ~Step() {
            if (fTrace == nullptr || !fTrace->fTraceTime) {
                return;
            }
            auto endTime = SkCycles::Now() - fStartTime;
            fTrace->addTrace(/*name=*/fName,
                             /*cycles=*/endTime,
                             /*scanlines=*/fScanlines,
                             /*pixels=*/fPixels);
        }
    private:
        SkBlitterTrace* fTrace = nullptr;
        const char* fName = "";
        uint64_t fStartTime = 0ul;
        uint64_t fScanlines = 0ul;
        uint64_t fPixels = 0ul;
    };

private:
    const char* fHeader = "";
    bool fTraceSteps = false;
    bool fTraceTime = false;
    uint64_t fCycles = 0ul;
    uint64_t fScanlines = 0ul;
    uint64_t fPixels = 0ul;
};

#endif
