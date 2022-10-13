/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "tests/graphite/RecordingSurfacesTest.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/mtl/MtlTypes.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {

DEF_GRAPHITE_TEST_FOR_METAL_CONTEXT(MtlRecordingSurfacesTest, reporter, context) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    MtlTextureInfo textureInfo;
    textureInfo.fLevelCount = 1;
    textureInfo.fFormat = MTLPixelFormatRGBA8Unorm;
    textureInfo.fUsage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    sk_sp<SkSurface> surface = SkSurface::MakeGraphiteFromBackendTexture(
            recorder.get(),
            recorder->createBackendTexture({64, 64}, textureInfo),
            kRGBA_8888_SkColorType,
            nullptr,
            nullptr);
    RunRecordingSurfacesTest(reporter, context, surface.get());
}

}  // namespace skgpu::graphite
