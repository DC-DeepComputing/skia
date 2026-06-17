/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Texture.h"

#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/MutableTextureState.h"
#include "src/core/SkEnumBitMask.h"
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
        , fMutableState(std::move(mutableState)) {
        fTextureUsages = usages;
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

void Texture::AppendUsagesToLabel(std::string label, SkEnumBitMask<TextureUsageFlags> usages) {
    if (usages == TextureUsageFlags::kNone) {
        return;
    }

    if (SkToBool(usages & TextureUsageFlags::kAsyncReadPixelsFallbackTexture)) {
        label += "_AsyncReadPixelsFallback";
    }
    if (SkToBool(usages & TextureUsageFlags::kChildDevice)) {
        label += "_ChildDevice";
    }
    if (SkToBool(usages & TextureUsageFlags::kCopyDeviceTexture)) {
        label += "_DeviceCopy";
    }
    if (SkToBool(usages & TextureUsageFlags::kDiscardableLoadMSAATexture)) {
        label += "_DiscardableLoadMSAA";
    }
    if (SkToBool(usages & TextureUsageFlags::kDispatchWriteOnlyStorageTexture)) {
        label += "_DispatchWriteOnlyStorage";
    }
    if (SkToBool(usages & TextureUsageFlags::kDstCopy)) {
        label += "_DstCopy";
    }
    if (SkToBool(usages & TextureUsageFlags::kEvalBlurTexture)) {
        label += "_EvalBlur";
    }
    if (SkToBool(usages & TextureUsageFlags::kGenerateMipmapsScratchTexture)) {
        label += "_GenerateMipmaps";
    }
    if (SkToBool(usages & TextureUsageFlags::kImageFilterCachedBitmap)) {
        label += "_ImageFilterCachedBitmap";
    }
    if (SkToBool(usages & TextureUsageFlags::kImageFilterResult)) {
        label += "_ImageFilterResult";
    }
    if (SkToBool(usages & TextureUsageFlags::kImageMakeCTandCSTexture)) {
        label += "_CTandCSConversion";
    }
    if (SkToBool(usages & TextureUsageFlags::kImageSubsetTexture)) {
        label += "_Subset";
    }
    if (SkToBool(usages & TextureUsageFlags::kLazySkImageBitmapTexture)) {
        label += "_LazySkImageBitmap";
    }
    if (SkToBool(usages & TextureUsageFlags::kRasterBitmapTexture)) {
        label += "_RasterBitmap";
    }
    if (SkToBool(usages & TextureUsageFlags::kRescaleDstTexture)) {
        label += "_RescaleDst";
    }
    if (SkToBool(usages & TextureUsageFlags::kRescaleImageTempTexture)) {
        label += "_RescaleImageTemp";
    }
    if (SkToBool(usages & TextureUsageFlags::kRescaleLinearGammaTexture)) {
        label += "_RescaleLinearGamma";
    }
    if (SkToBool(usages & TextureUsageFlags::kYUVRasterBitmapPlane)) {
        label += "_YUVRasterBitmapPlane";
    }
}

void Texture::setLabel(std::string_view label, SkEnumBitMask<TextureUsageFlags> textureUsages) {
    if (fLabel == label && fTextureUsages == textureUsages) {
        return;
    }

    fTextureUsages = textureUsages;

    std::string labelWithUsages {label};
    AppendUsagesToLabel(labelWithUsages, fTextureUsages);

    fLabel = labelWithUsages;

    fBackendLabelDirty = true;
}

} // namespace skgpu::graphite
