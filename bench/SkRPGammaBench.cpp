/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkString.h"
#include "include/private/SkAlign.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"

#include <memory>

class SkRPGammaBench : public Benchmark {
public:
    SkRPGammaBench(float exponent) : fExponent(exponent) {
        fName.printf("skrp_gamma_%.2f", fExponent);
    }

protected:
    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

    void onDraw(int loops, SkCanvas*) override {
        SkArenaAlloc alloc(1024);
        SkRasterPipeline p(&alloc);

        // Load a solid color context to have values in registers r, g, b, a
        static float color[4] = {0.25f, 0.5f, 0.75f, 1.0f};
        p.append(SkRasterPipelineOp::uniform_color, color);

        p.append(SkRasterPipelineOp::gamma_, &fExponent);

        for (int i = 0; i < loops; i++) {
            p.run(0, 0, 1024, 1);
        }
    }

private:
    float fExponent;
    SkString fName;
};

DEF_BENCH(return new SkRPGammaBench(2.00f);)
DEF_BENCH(return new SkRPGammaBench(0.50f);)
DEF_BENCH(return new SkRPGammaBench(1.00f);)
DEF_BENCH(return new SkRPGammaBench(2.20f);)
