/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitterTrace_DEFINED
#define SkBlitterTrace_DEFINED

#include <unordered_map>
#include "src/utils/SkCycles.h"

struct str_eq
{
    bool operator()(const char* lhs, const char* rhs) const
    {
        return strcmp(lhs, rhs) == 0;
    }
};
struct str_hash
{
    size_t operator()(const char* str) const
    {
        // djb2
        unsigned long hash = 5381;
        int c;

        while ((c = *str++) != 0)
            hash = ((hash << 5) + hash) + c;

        return hash;
    }
};

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
    void addCounts(const char* label, uint64_t incr) { fLabeledCounts[label] += incr; }
    void reset() {
        fCycles = 0ul;
        fScanlines = 0ul;
        fPixels = 0ul;
        fLabeledCounts.clear();
    }

    void printIncrements(const char* name, uint64_t cycles, uint64_t scanLines, uint64_t pixels) const {
        SkDebugf("%s %s: cycles=%lu+%lu scanlines=%lu+%lu pixels=%lu",
                 fHeader, name,
                 fCycles - cycles, cycles,
                 fScanlines - scanLines, scanLines,
                 fPixels);
        for (auto& pair : fLabeledCounts) {
            SkDebugf(" %s=%lu", pair.first, pair.second);
        }
        SkDebugf("\n");
    }

    void printCounts(const char* name) const {
        SkDebugf("%s %s: cycles=%lu scanlines=%lu pixels=%lu", fHeader, name, fCycles, fScanlines,
                 fPixels);
        for (auto& pair : fLabeledCounts) {
            SkDebugf(" %s=%lu", pair.first, pair.second);
        }
        SkDebugf("\n");
    }

    uint64_t getCycles() const { return fCycles; }
    uint64_t getScanlines() const { return fScanlines; }
    uint64_t getPixels() const { return fPixels; }
    uint64_t getCount(const char* label) { return fLabeledCounts[label]; }

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
    uint64_t fCycles = 0ul;
    uint64_t fScanlines = 0ul;
    uint64_t fPixels = 0ul;
    std::unordered_map<const char*, uint64_t, str_hash, str_eq> fLabeledCounts;
};

#endif