/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "bench/Benchmark.h"
#include "include/core/SkString.h"
#include "include/private/base/SkMutex.h"
#include "src/base/SkSpinlock.h"

template <typename Mutex>
class MutexBench : public Benchmark {
public:
    MutexBench(SkString benchPrefix) : fBenchName(benchPrefix += "UncontendedBenchmark") { }
    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kNonRendering;
    }

protected:
    const char* onGetName() override {
        return fBenchName.c_str();
    }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            fMu.acquire();
            fMu.release();
        }
    }

private:
    SkString fBenchName;
    Mutex fMu;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new MutexBench<SkMutex>(SkString("SkMutex")); )
DEF_BENCH( return new MutexBench<SkSpinlock>(SkString("SkSpinlock")); )
