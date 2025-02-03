/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/TextureInfo.h"

#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/graphite/TextureInfoPriv.h"

namespace skgpu::graphite {

TextureInfo::TextureInfo(){};
TextureInfo::~TextureInfo() = default;

TextureInfo::TextureInfo(const TextureInfo& that) {
    if (!that.isValid()) {
        SkASSERT(!this->isValid());
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
    if (!this->isValid() && !that.isValid()) {
        return true;
    }
    if (this->backend() != that.backend()) {
        return false;
    }

    return fData->equals(that);
}

bool TextureInfo::isCompatible(const TextureInfo& that) const {
    if (!this->isValid() || !that.isValid()) {
        return false;
    }

    if (this->backend() != that.backend()) {
        return false;
    }

    return fData->isCompatible(that);
}

SkString TextureInfo::toString() const {
    if (!this->isValid()) {
        return SkString("{}");
    }

    // Strip the leading "k" from the enum name when creating the TextureInfo string.
    SkASSERT(BackendApiToStr(this->backend())[0] == 'k');
    const char* backendName = BackendApiToStr(this->backend()) + 1;
    return SkStringPrintf("%s(%s,bpp=%zu,sampleCount=%u,mipmapped=%d)",
                          backendName,
                          fData->toBackendString().c_str(),
                          this->bytesPerPixel(),
                          fData->fSampleCount,
                          static_cast<int>(fData->fMipmapped));
}

SkString TextureInfo::toRPAttachmentString() const {
    if (!this->isValid()) {
        return SkString("{}");
    }

    // Strip the leading "k" from the enum name when creating the TextureInfo string.
    SkASSERT(BackendApiToStr(this->backend())[0] == 'k');
    const char* backendName = BackendApiToStr(this->backend()) + 1;
    // For renderpass attachments, the string will contain the view format and sample count only
    return SkStringPrintf("%s(f=%u,s=%u)",
                          backendName,
                          fData->viewFormat(),
                          fData->fSampleCount);
}

} // namespace skgpu::graphite
