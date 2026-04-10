/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkPath.h"
#include "include/core/SkMatrix.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"
#include "src/gpu/graphite/sparse_strips/StripGenerator.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"

#include "tests/graphite/sparse_strips/compute/TestUtilsCompute.h"
#include "tools/graphite/GraphiteTestContext.h"

using namespace skgpu::graphite;

DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(SparseStrips_Compute_CoverageDebug,
                                   reporter,
                                   context,
                                   testContext) {
    constexpr uint32_t debugCount = 2048;

    // Skip for D3D11 Dawn context just like the clip tests
    if (testContext->contextType() == skgpu::ContextType::kDawn_D3D11) { return; }

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    // 1. Create a dummy MSAA LUT
    // Adjust the size based on what your shader/config actually expects.
    const SkTDArray<uint8_t> msaaLUT = GenerateMSAALUT<uint8_t>();

    // 2. Initialize the StripGenerator with GpuTile<4, 4>
    // Using a 32x32 device size to give the path some breathing room.
    SkISize deviceSize = SkISize::Make(32, 32);
    StripGenerator<GpuTile<4, 4>> generator(deviceSize, msaaLUT, recorder.get());

    // 3. Create the extremely simple diamond path (rotated rect with 8-wide edges)
    // A rect from -4 to 4 gives an edge length of 8.
    SkPath path = SkPath::Rect(SkRect::MakeLTRB(0, 0, 8, 8));

    // Rotate 45 degrees to create the diamond, then translate to the center of the device bounds.
    SkMatrix ctm = SkMatrix::RotateDeg(45.f);
    ctm.postTranslate(16.f, 16.f);

    SkIRect clip = SkIRect::MakeSize(deviceSize);

    // 4. Process the geometry into GPU tiles
    generator.processGeometry(path, ctm, clip);

    // 5. Trigger your custom GPU flush to build and dispatch the compute task
    // We pass a lambda to allocate the scratch buffer exactly how we need it for debugging.
    auto allocDebugBuf = [](DrawBufferManager* bufMgr) -> BindBufferInfo {
        size_t debugSize = debugCount * sizeof(uint32_t); // Or however big you need it
        return bufMgr->getScratchStorage(debugSize).getSubrange(debugSize, 1);
    };

    BindBufferInfo debugInfo = generator.flushGpu(allocDebugBuf);

    if (!debugInfo.fBuffer) {
        ERRORF(reporter, "Failed to retrieve debug buffer info");
        return;
    }

    // 6. Queue a copy task from the GPU scratch buffer to a CPU-readable buffer.
    auto readbackBuffer = sync_buffer_to_cpu(recorder.get(), debugInfo.fBuffer);

    // 7. Snap the recording, insert it, and submit (syncing to CPU)
    // This executes the compute shader and the copy command, blocking until done.
    submit_recording(context, testContext, recorder.get());

    // 8. Map the readback buffer
    uint32_t* debugData = static_cast<uint32_t*>(
            map_buffer(context, testContext, readbackBuffer.get(), debugInfo.fOffset));

    SkASSERT(readbackBuffer->isMapped() && debugData != nullptr);

    // 9. Print out the raw 8-bit masks!
    printf("--- Compute Shader Raw 8-bit Sample Masks ---\n");

    // For a 4x4 tile, we have 16 pixels.
    // If you have multiple tiles, adjust this bound (num_tiles * 16).
    int numPixelsToPrint = 16;

    for (int p = 0; p < 32; ++p) {
        printf("Pixel[%2d] Samples: ", p);
        for (int s = 0; s < 8; ++s) {
            uint32_t sampleVal = debugData[p * 8 + s];
            // %02X prints nicely formatted hex (e.g. 0x80, 0x00, 0x81)
            printf("%02X ", sampleVal);
        }
        printf("\n");
    }

    // Clean up
    if (readbackBuffer->isMapped()) {
        readbackBuffer->unmap();
    }
}