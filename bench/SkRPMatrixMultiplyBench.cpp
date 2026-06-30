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
#include "src/core/SkRasterPipelineContextUtils.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"

#include <memory>

class SkRPMatrixMultiplyBench : public Benchmark {
public:
    SkRPMatrixMultiplyBench(int N, int outColumns, int outRows)
        : fN(N), fOutColumns(outColumns), fOutRows(outRows) {
        fName.printf("skrp_matrix_multiply_%dx%d_by_%dx%d", fN, fOutRows, fOutColumns, fN);

        // Align fSlots to 64 bytes for AVX-512 in SkRasterPipeline.
        fStorage.reset(new char[256 * sizeof(float) + 64]);
        fSlots = reinterpret_cast<float*>(
                SkAlignTo(reinterpret_cast<uintptr_t>(fStorage.get()), (uintptr_t)64));

        // Initialize mock matrix elements to non-zero values
        for (int i = 0; i < 256; i++) {
            fSlots[i] = (float)(i + 1) * 0.1f;
        }
    }

protected:
    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

    void onDraw(int loops, SkCanvas*) override {
        SkArenaAlloc alloc(1024);
        SkRasterPipeline p(&alloc);

        SkRasterPipelineContexts::MatrixMultiplyCtx ctx;
        ctx.dst = 0;
        ctx.leftColumns = fN;
        ctx.leftRows = fOutRows;
        ctx.rightColumns = fOutColumns;
        ctx.rightRows = fN;

        p.append(SkRasterPipelineOp::set_base_pointer, fSlots);

        SkRasterPipelineOp op;
        if (fN == 2) {
            op = SkRasterPipelineOp::matrix_multiply_2;
        } else if (fN == 3) {
            op = SkRasterPipelineOp::matrix_multiply_3;
        } else if (fN == 4) {
            op = SkRasterPipelineOp::matrix_multiply_4;
        } else {
            SkUNREACHABLE;
        }

        p.append(op, SkRPCtxUtils::Pack(ctx, &alloc));

        for (int i = 0; i < loops; i++) {
            p.run(0, 0, 128, 1);
        }
    }

private:
    int fN;
    int fOutColumns;
    int fOutRows;
    SkString fName;
    std::unique_ptr<char[]> fStorage;
    float* fSlots = nullptr;
};

// Test standard transform cases
DEF_BENCH(return new SkRPMatrixMultiplyBench(4, 1, 4);)  // 4x4 matrix by 4x1 vector (X, Y, Z, W)
DEF_BENCH(return new SkRPMatrixMultiplyBench(4, 4, 4);)  // 4x4 matrix by 4x4 matrix
DEF_BENCH(return new SkRPMatrixMultiplyBench(3, 1, 3);)  // 3x3 matrix by 3x1 vector (X, Y, W)
DEF_BENCH(return new SkRPMatrixMultiplyBench(3, 3, 3);)  // 3x3 matrix by 3x3 matrix
