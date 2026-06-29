/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkString.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"

class SkRPDivBench : public Benchmark {
public:
    enum class Type { kInt, kUint };

    SkRPDivBench(Type type, int slots) : fType(type), fSlots(slots) {
        fName.printf("skrp_div_%s_%d", fType == Type::kInt ? "int" : "uint", fSlots);
    }

protected:
    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < 1024; i++) fData[i] = i + 1;

        SkArenaAlloc alloc(1024);
        SkRasterPipeline p(&alloc);

        SkRasterPipelineOp op;
        if (fType == Type::kInt) {
            switch (fSlots) {
                case 1: op = SkRasterPipelineOp::div_int; break;
                case 4: op = SkRasterPipelineOp::div_4_ints; break;
                default: SkUNREACHABLE;
            }
        } else {
            switch (fSlots) {
                case 1: op = SkRasterPipelineOp::div_uint; break;
                case 4: op = SkRasterPipelineOp::div_4_uints; break;
                default: SkUNREACHABLE;
            }
        }
        p.append(op, fData);

        for (int i = 0; i < loops; i++) {
            p.run(0, 0, 128, 1);
        }
    }

private:
    Type fType;
    int fSlots;
    SkString fName;
    alignas(64) int32_t fData[1024];
};

DEF_BENCH(return new SkRPDivBench(SkRPDivBench::Type::kInt, 1);)
DEF_BENCH(return new SkRPDivBench(SkRPDivBench::Type::kInt, 4);)
DEF_BENCH(return new SkRPDivBench(SkRPDivBench::Type::kUint, 1);)
DEF_BENCH(return new SkRPDivBench(SkRPDivBench::Type::kUint, 4);)
