/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrBackendUtils.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/DataUtils.h"
#include "src/gpu/ganesh/GrBackendSurfacePriv.h"

SkTextureCompressionType GrBackendFormatToCompressionType(const GrBackendFormat& format) {
    switch (format.backend()) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kMock:     [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return GrBackendSurfacePriv::GetBackendData(format)->compressionType();
        case GrBackendApi::kUnsupported: {
            break;
        }
    }
    return SkTextureCompressionType::kNone;
}

size_t GrBackendFormatBytesPerBlock(const GrBackendFormat& format) {
    switch (format.backend()) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kMock:     [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return GrBackendSurfacePriv::GetBackendData(format)->bytesPerBlock();
        case GrBackendApi::kUnsupported: {
            break;
        }
    }
    return 0;
}

size_t GrBackendFormatBytesPerPixel(const GrBackendFormat& format) {
    if (GrBackendFormatToCompressionType(format) != SkTextureCompressionType::kNone) {
        return 0;
    }
    return GrBackendFormatBytesPerBlock(format);
}

int GrBackendFormatStencilBits(const GrBackendFormat& format) {
    switch (format.backend()) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kMock:     [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return GrBackendSurfacePriv::GetBackendData(format)->stencilBits();
        case GrBackendApi::kUnsupported: {
            break;
        }
    }
    return 0;
}
