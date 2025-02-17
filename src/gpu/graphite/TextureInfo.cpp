/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/TextureInfo.h"

#include "include/core/SkFourByteTag.h"
#include "include/core/SkStream.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/TextureInfoPriv.h"

namespace skgpu::graphite {

TextureInfo::TextureInfo(const TextureInfo& that)
        : fBackend(that.fBackend)
        , fProtected(that.fProtected)
        , fMemoryless(that.fMemoryless) {
    if (!that.fData.has_value()) {
        SkASSERT(!fData.has_value());
        return;
    }

    that.fData->copyTo(fData);
}

TextureInfo& TextureInfo::operator=(const TextureInfo& that) {
    if (this != &that) {
        this->~TextureInfo();
        new (this) TextureInfo(that);
    }
    return *this;
}

bool TextureInfo::operator==(const TextureInfo& that) const {
    if (fBackend != that.fBackend) {
        return false;
    } else if (fBackend == skgpu::BackendApi::kUnsupported) {
        SkASSERT(!fData.has_value() && !that.fData.has_value());
        return true;
    } else {
        SkASSERT(fData.has_value() && that.fData.has_value());
        return fData->fSampleCount == that.fData->fSampleCount &&
               fData->fMipmapped == that.fData->fMipmapped &&
               fData->equals(that);
    }
}

bool TextureInfo::isCompatible(const TextureInfo& that) const {
    if (fBackend != that.fBackend) {
        return false;
    } else if (fBackend == skgpu::BackendApi::kUnsupported) {
        SkASSERT(!fData.has_value() && !that.fData.has_value());
        return true;
    } else {
        SkASSERT(fData.has_value() && that.fData.has_value());
        return fData->fSampleCount == that.fData->fSampleCount &&
               fData->fMipmapped == that.fData->fMipmapped &&
               fData->isCompatible(that);
    }
}

SkString TextureInfo::toString() const {
    if (!this->isValid()) {
        return SkString("{}");
    }

    // Strip the leading "k" from the enum name when creating the TextureInfo string.
    SkASSERT(BackendApiToStr(fBackend)[0] == 'k');
    const char* backendName = BackendApiToStr(fBackend) + 1;

    return SkStringPrintf("%s(%s,bpp=%zu,sampleCount=%u,mipmapped=%d,protected=%d)",
                          backendName,
                          fData->toBackendString().c_str(),
                          this->bytesPerPixel(),
                          fData->fSampleCount,
                          static_cast<int>(fData->fMipmapped),
                          static_cast<int>(fProtected));
}

SkString TextureInfoPriv::GetAttachmentLabel(const TextureInfo& info) {
    if (!info.isValid()) {
        return SkString("{}");
    }

    // Strip the leading "k" from the enum name when creating the TextureInfo string.
    SkASSERT(BackendApiToStr(info.backend())[0] == 'k');
    const char* backendName = BackendApiToStr(info.backend()) + 1;
    // For renderpass attachments, the string will contain the view format and sample count only
    return SkStringPrintf("%s(f=%u,s=%u)",
                          backendName,
                          info.fData->viewFormat(),
                          info.fData->fSampleCount);
}

uint32_t TextureInfoPriv::GetInfoTag(const TextureInfo& info) {
    SkASSERT(info.isValid());
    return SkSetFourByteTag(255, // force value to be non-zero
                            SkTo<uint8_t>(info.backend()),
                            SkTo<uint8_t>(info.mipmapped()),
                            SkTo<uint8_t>(info.numSamples()));
}

std::tuple<BackendApi, Mipmapped, int> TextureInfoPriv::ParseInfoTag(uint32_t tag) {
    static constexpr std::tuple<BackendApi, Mipmapped, int> kInvalid =
            {BackendApi::kUnsupported, Mipmapped::kNo, 0};

    uint8_t fixed     = 0xFF & (tag >> 24);
    uint8_t backend   = 0xFF & (tag >> 16);
    uint8_t mipmapped = 0xFF & (tag >>  8);
    uint8_t samples   = 0xFF & (tag >>  0);
    if (fixed != 255) {
        return kInvalid;
    }
    if (backend >= SkTo<uint8_t>(BackendApi::kUnsupported)) {
        return kInvalid;
    }
    return {static_cast<BackendApi>(backend),
            mipmapped ? Mipmapped::kYes : Mipmapped::kNo,
            static_cast<int>(samples)};
}

} // namespace skgpu::graphite
