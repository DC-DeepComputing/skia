/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/mock/GrMockCaps.h"

#include "include/gpu/ganesh/mock/GrMockBackendSurface.h"
#include "include/private/base/SkDebug.h"
#include "src/base/SkMathPriv.h"
#include "src/gpu/ganesh/GrProgramDesc.h"
#include "src/gpu/ganesh/TestFormatColorTypeCombination.h"

int GrMockCaps::getRenderTargetSampleCount(int requestCount, GrColorType ct) const {
    requestCount = std::max(requestCount, 1);

    switch (fOptions.fConfigOptions[(int)ct].fRenderability) {
        case GrMockOptions::ConfigOptions::Renderability::kNo:
            return 0;
        case GrMockOptions::ConfigOptions::Renderability::kNonMSAA:
            return requestCount > 1 ? 0 : 1;
        case GrMockOptions::ConfigOptions::Renderability::kMSAA:
            return requestCount > kMaxSampleCnt ? 0 : SkNextPow2(requestCount);
    }
    return 0;
}

GrProgramDesc GrMockCaps::makeDesc(GrRenderTarget* /* rt */,
                                   const GrProgramInfo& programInfo,
                                   ProgramDescOverrideFlags overrideFlags) const {
    SkASSERT(overrideFlags == ProgramDescOverrideFlags::kNone);
    GrProgramDesc desc;
    GrProgramDesc::Build(&desc, programInfo, *this);
    return desc;
}

uint64_t GrMockCaps::computeFormatKey(const GrBackendFormat& format) const {
#ifdef SK_DEBUG
    SkTextureCompressionType compression = GrBackendFormats::AsMockCompressionType(format);
    SkASSERT(compression == SkTextureCompressionType::kNone);
#endif
    auto ct = GrBackendFormats::AsMockColorType(format);
    return (uint64_t)ct;
}

#if defined(GPU_TEST_UTILS)
std::vector<GrTest::TestFormatColorTypeCombination> GrMockCaps::getTestingCombinations() const {
    auto mockForGrColorType = [](GrColorType ct) {
        return GrBackendFormats::MakeMock(ct, SkTextureCompressionType::kNone);
    };
    auto mockForCompressionType = [](SkTextureCompressionType ct) {
        return GrBackendFormats::MakeMock(GrColorType::kUnknown, ct);
    };
    using CompressionType = SkTextureCompressionType;
    // TODO: need to add compressed formats to this list
    std::vector<GrTest::TestFormatColorTypeCombination> combos = {
        { GrColorType::kAlpha_8,          mockForGrColorType(GrColorType::kAlpha_8)},
        { GrColorType::kBGR_565,          mockForGrColorType(GrColorType::kBGR_565)},
        { GrColorType::kRGB_565,          mockForGrColorType(GrColorType::kRGB_565)},
        { GrColorType::kABGR_4444,        mockForGrColorType(GrColorType::kABGR_4444)},
        { GrColorType::kRGBA_8888,        mockForGrColorType(GrColorType::kRGBA_8888)},
        { GrColorType::kRGBA_8888_SRGB,   mockForGrColorType(GrColorType::kRGBA_8888_SRGB)},
        { GrColorType::kRGB_888x,         mockForGrColorType(GrColorType::kRGB_888x)},
        { GrColorType::kRG_88,            mockForGrColorType(GrColorType::kRG_88)},
        { GrColorType::kBGRA_8888,        mockForGrColorType(GrColorType::kBGRA_8888)},
        { GrColorType::kRGBA_1010102,     mockForGrColorType(GrColorType::kRGBA_1010102)},
        { GrColorType::kBGRA_1010102,     mockForGrColorType(GrColorType::kBGRA_1010102)},
        { GrColorType::kGray_8,           mockForGrColorType(GrColorType::kGray_8)},
        { GrColorType::kAlpha_F16,        mockForGrColorType(GrColorType::kAlpha_F16)},
        { GrColorType::kRGBA_F16,         mockForGrColorType(GrColorType::kRGBA_F16)},
        { GrColorType::kRGBA_F16_Clamped, mockForGrColorType(GrColorType::kRGBA_F16_Clamped)},
        { GrColorType::kRGB_F16F16F16x,   mockForGrColorType(GrColorType::kRGB_F16F16F16x)},
        { GrColorType::kAlpha_16,         mockForGrColorType(GrColorType::kAlpha_16)},
        { GrColorType::kRG_1616,          mockForGrColorType(GrColorType::kRG_1616)},
        { GrColorType::kRGBA_16161616,    mockForGrColorType(GrColorType::kRGBA_16161616)},
        { GrColorType::kRG_F16,           mockForGrColorType(GrColorType::kRG_F16)},
        // For these two compressed image formats the color type will effectively be RGB_888x
        { GrColorType::kRGB_888x,     mockForCompressionType(CompressionType::kETC2_RGB8_UNORM)},
        { GrColorType::kRGB_888x,     mockForCompressionType(CompressionType::kBC1_RGB8_UNORM)},
        { GrColorType::kRGBA_8888,    mockForCompressionType(CompressionType::kBC1_RGBA8_UNORM)},
    };

#ifdef SK_DEBUG
    for (const GrTest::TestFormatColorTypeCombination& combo : combos) {
        SkASSERT(this->onAreColorTypeAndFormatCompatible(combo.fColorType, combo.fFormat));
    }
#endif

    return combos;
}

#endif
