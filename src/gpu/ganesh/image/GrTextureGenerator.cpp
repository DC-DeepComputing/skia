/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/ganesh/GrTextureGenerator.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorType.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkSize.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkNextID.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"

#include <utility>

GrTextureGenerator::GrTextureGenerator(const SkImageInfo& info, uint32_t uniqueID)
        : SkImageGenerator(info, uniqueID) {}

GrSurfaceProxyView GrTextureGenerator::generateTexture(GrRecordingContext* ctx,
                                                       const SkImageInfo& info,
                                                       GrMipmapped mipmapped,
                                                       GrImageTexGenPolicy texGenPolicy) {
    SkASSERT_RELEASE(fInfo.dimensions() == info.dimensions());

    if (!ctx || ctx->abandoned()) {
        return {};
    }

    return this->onGenerateTexture(ctx, info, mipmapped, texGenPolicy);
}
