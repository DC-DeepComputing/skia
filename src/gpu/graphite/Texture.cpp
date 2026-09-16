/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Texture.h"

#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/MutableTextureState.h"
#include "include/private/SkEnumBitMask.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/TextureUtils.h"

namespace skgpu::graphite {

Texture::Texture(const SharedContext* sharedContext,
                 SkISize dimensions,
                 const TextureInfo& info,
                 bool isTransient,
                 sk_sp<MutableTextureState> mutableState,
                 Ownership ownership,
                 std::string_view label,
                 SkEnumBitMask<TextureUsageFlags> usages)
        // For the initial GPU size, this assumes that a transient texture will not have any actual
        // memory. Over a texture's lifetime this may not stay the case.
        : Resource(sharedContext, ownership, isTransient ? 0 : ComputeSize(dimensions, info))
        , fDimensions(dimensions)
        , fInfo(info)
        , fMutableState(std::move(mutableState))
        , fTextureUsages(usages) {
        // Explicitly going through Resource::setLabel(...) processes `label` and the usage flags at
        // once without having to modify `Resource` to accept Texture-specific usage flags as args
        // or do more complicated code gymnastics.
        this->setLabel(label, usages);
        this->synchronizeBackendLabel();
}

Texture::~Texture() = default;

void Texture::setReleaseCallback(sk_sp<RefCntedCallback> releaseCallback) {
    fReleaseCallback = std::move(releaseCallback);
}

void Texture::invokeReleaseProc() {
    if (fReleaseCallback) {
        // Depending on the ref count of fReleaseCallback this may or may not actually trigger
        // the ReleaseProc to be called.
        fReleaseCallback.reset();
    }
}

bool Texture::uploadDataOnHost(const UploadSource& source) {
    SK_ABORT("Not implemented");
}

MutableTextureState* Texture::mutableState() const { return fMutableState.get(); }

void Texture::onDumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump,
                                     const char* dumpName) const {
    SkString dimensionsStr;
    dimensionsStr.printf("(%dx%d)", fDimensions.width(), fDimensions.height());
    traceMemoryDump->dumpStringValue(dumpName, "dimensions", dimensionsStr.c_str());
    traceMemoryDump->dumpStringValue(dumpName, "textureInfo", fInfo.toString().c_str());
}

namespace {
std::string usage_flag_to_str(TextureUsageFlags flag) {
    switch (flag) {
        case TextureUsageFlags::kNone:                     return "";
        case TextureUsageFlags::kAsyncReadPixelsFallback:  return "AsyncReadPixelsFallback";
        case TextureUsageFlags::kChildDevice:              return "ChildDevice";
        case TextureUsageFlags::kCopyDevice:               return "DeviceCopy";
        case TextureUsageFlags::kDiscardableLoadMSAA:      return "DiscardableLoadMSAA";
        case TextureUsageFlags::kDispatchWriteOnlyStorage: return "DispatchWriteOnlyStorage";
        case TextureUsageFlags::kDstCopy:                  return "DstCopy";
        case TextureUsageFlags::kEvalBlur:                 return "EvalBlur";
        case TextureUsageFlags::kGenerateMipmapsScratch:   return "GenerateMipmaps";
        case TextureUsageFlags::kImageFilterCachedBitmap:  return "ImageFilterCachedBitmap";
        case TextureUsageFlags::kImageFilterResult:        return "ImageFilterResult";
        case TextureUsageFlags::kImageMakeCTandCS:         return "CTandCSConversion";
        case TextureUsageFlags::kImageSubset:              return "Subset";
        case TextureUsageFlags::kLazySkImageBitmap:        return "LazySkImageBitmap";
        case TextureUsageFlags::kRasterBitmap:             return "RasterBitmap";
        case TextureUsageFlags::kRescaleDst:               return "RescaleDst";
        case TextureUsageFlags::kRescaleImageTemp:         return "RescaleImageTemp";
        case TextureUsageFlags::kRescaleLinearGamma:       return "RescaleLinearGamma";
        case TextureUsageFlags::kYUVRasterBitmapPlane:     return "YUVRasterBitmapPlane";
    }
    SkUNREACHABLE;
    return "";
}
} // anonymous namespace

void Texture::AppendUsagesToLabel(std::string label, SkEnumBitMask<TextureUsageFlags> usageMask) {
    if (usageMask == TextureUsageFlags::kNone) {
        return;
    }

    for (int bitIdx = 0;
         TextureUsageFlags(1 << bitIdx) <= TextureUsageFlags::kLastUsageFlag; bitIdx++) {
        auto flag = TextureUsageFlags(1 << bitIdx);
        if (SkToBool(usageMask & flag)) {
            label += "_";
            label += usage_flag_to_str(flag);
        }
    }
}

} // namespace skgpu::graphite
