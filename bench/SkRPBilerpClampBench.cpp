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

class SkRPBilerpClampBench : public Benchmark {
public:
    SkRPBilerpClampBench() {
        fName.printf("skrp_bilerp_clamp_8888_micro");

        // Align buffers to 64 bytes (critical for AVX-512 in SkRasterPipeline).
        // 1) Source texture: 64x64 RGBA8888 pixels = 4096 uint32_t.
        fSrcStorage.reset(new char[64 * 64 * sizeof(uint32_t) + 64]);
        fSrcPixels = reinterpret_cast<uint32_t*>(
                SkAlignTo(reinterpret_cast<uintptr_t>(fSrcStorage.get()), (uintptr_t)64));

        // Fill source texture with dummy non-zero pixel data.
        for (int i = 0; i < 64 * 64; ++i) {
            fSrcPixels[i] = 0xFF00FF00 | i;
        }

        // Set up GatherCtx
        fGatherCtx.pixels = fSrcPixels;
        fGatherCtx.stride = 64;
        fGatherCtx.width = 64.0f;
        fGatherCtx.height = 64.0f;
        fGatherCtx.roundDownAtInteger = false;

        // 2) Destination buffer: 1024 RGBA8888 pixels.
        fDstStorage.reset(new char[1024 * sizeof(uint32_t) + 64]);
        fDstPixels = reinterpret_cast<uint32_t*>(
                SkAlignTo(reinterpret_cast<uintptr_t>(fDstStorage.get()), (uintptr_t)64));

        // Set up MemoryCtx (for store_8888)
        fStoreCtx.pixels = fDstPixels;
        fStoreCtx.stride = 1024;
    }

protected:
    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

    void onDraw(int loops, SkCanvas*) override {
        SkArenaAlloc alloc(1024);
        SkRasterPipeline p(&alloc);

        p.append(SkRasterPipelineOp::seed_shader);
        p.append(SkRasterPipelineOp::bilerp_clamp_8888, &fGatherCtx);
        p.append(SkRasterPipelineOp::store_8888, &fStoreCtx);

        for (int i = 0; i < loops; i++) {
            p.run(0, 0, 1024, 1);
        }
    }

private:
    SkString fName;
    std::unique_ptr<char[]> fSrcStorage;
    uint32_t* fSrcPixels = nullptr;
    SkRasterPipelineContexts::GatherCtx fGatherCtx;

    std::unique_ptr<char[]> fDstStorage;
    uint32_t* fDstPixels = nullptr;
    SkRasterPipelineContexts::MemoryCtx fStoreCtx;
};

DEF_BENCH(return new SkRPBilerpClampBench();)
