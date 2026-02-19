/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/ganesh/GrBackendSurface.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/MutableTextureState.h"  // IWYU pragma: keep
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/ganesh/GrBackendSurfacePriv.h"
#include "src/gpu/ganesh/GrUtil.h"

#include <algorithm>
#include <new>

GrBackendFormat::GrBackendFormat() : fValid(false) {}
GrBackendFormat::~GrBackendFormat() = default;

GrBackendFormat::GrBackendFormat(const GrBackendFormat& that)
        : fBackend(that.fBackend)
        , fValid(that.fValid)
        , fTextureType(that.fTextureType) {
    if (!fValid) {
        return;
    }

    switch (fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kMock:     [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            fFormatData.reset();
            that.fFormatData->copyTo(fFormatData);
            break;  // fFormatData is sufficient
        default:
            SK_ABORT("Unknown GrBackend");
    }
}

GrBackendFormat& GrBackendFormat::operator=(const GrBackendFormat& that) {
    if (this != &that) {
        this->~GrBackendFormat();
        new (this) GrBackendFormat(that);
    }
    return *this;
}

uint32_t GrBackendFormat::channelMask() const {
    if (!this->isValid()) {
        return 0;
    }
    switch (fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kMock:     [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return fFormatData->channelMask();

        default:
            return 0;
    }
}

GrColorFormatDesc GrBackendFormat::desc() const {
    if (!this->isValid()) {
        return GrColorFormatDesc::MakeInvalid();
    }
    switch (fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kMock:     [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return fFormatData->desc();

        default:
            return GrColorFormatDesc::MakeInvalid();
    }
}

GrBackendFormat GrBackendFormat::makeTexture2D() const {
    GrBackendFormat copy = *this;
    // TODO(b/293490566): Remove this kVulkan check once mock backend is using fFormatData.
    if (fBackend == GrBackendApi::kVulkan) {
        copy.fFormatData->makeTexture2D();
    }
    copy.fTextureType = GrTextureType::k2D;
    return copy;
}

bool GrBackendFormat::operator==(const GrBackendFormat& that) const {
    // Invalid GrBackendFormats are never equal to anything.
    if (!fValid || !that.fValid) {
        return false;
    }

    if (fBackend != that.fBackend) {
        return false;
    }

    switch (fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kMock:     [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return fFormatData->equal(that.fFormatData.get());
        default:
            SK_ABORT("Unknown GrBackend");
    }
}

#if defined(SK_DEBUG) || defined(GPU_TEST_UTILS)
#include "include/core/SkString.h"

SkString GrBackendFormat::toStr() const {
    SkString str;

    if (!fValid) {
        str.append("invalid");
        return str;
    }

    str.appendf("%s-", GrBackendApiToStr(fBackend));

    switch (fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kMock:     [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            str.append(fFormatData->toString());
            break;
        case GrBackendApi::kUnsupported:
            break;
    }

    return str;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
GrBackendTexture::GrBackendTexture() : fIsValid(false) {}

GrBackendTexture::~GrBackendTexture() = default;

GrBackendTexture::GrBackendTexture(const GrBackendTexture& that) : fIsValid(false) {
    *this = that;
}

GrBackendTexture& GrBackendTexture::operator=(const GrBackendTexture& that) {
    if (this == &that) {
        return *this;
    }

    fTextureData.reset();
    if (!that.isValid()) {
        fIsValid = false;
        return *this;
    }
    fWidth = that.fWidth;
    fHeight = that.fHeight;
    fMipmapped = that.fMipmapped;
    fBackend = that.fBackend;
    fTextureType = that.fTextureType;

    switch (that.fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kMock:     [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            that.fTextureData->copyTo(fTextureData);
            break;
        default:
            SK_ABORT("Unknown GrBackend");
    }
    fIsValid = true;
    return *this;
}

sk_sp<skgpu::MutableTextureState> GrBackendTexture::getMutableState() const {
    return fTextureData->getMutableState();
}

void GrBackendTexture::setMutableState(const skgpu::MutableTextureState& state) {
    fTextureData->setMutableState(state);
}

bool GrBackendTexture::isProtected() const {
    if (!this->isValid()) {
        return false;
    }
    return fTextureData->isProtected();
}

bool GrBackendTexture::isSameTexture(const GrBackendTexture& that) {
    if (!this->isValid() || !that.isValid()) {
        return false;
    }
    if (fBackend != that.fBackend) {
        return false;
    }
    switch (fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kMock:     [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return fTextureData->isSameTexture(that.fTextureData.get());
        default:
            return false;
    }
}

GrBackendFormat GrBackendTexture::getBackendFormat() const {
    if (!this->isValid()) {
        return GrBackendFormat();
    }
    switch (fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kMock:     [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return fTextureData->getBackendFormat();
        default:
            return GrBackendFormat();
    }
}

#if defined(GPU_TEST_UTILS)
bool GrBackendTexture::TestingOnly_Equals(const GrBackendTexture& t0, const GrBackendTexture& t1) {
    if (!t0.isValid() || !t1.isValid()) {
        return false; // two invalid backend textures are not considered equal
    }

    if (t0.fWidth != t1.fWidth ||
        t0.fHeight != t1.fHeight ||
        t0.fMipmapped != t1.fMipmapped ||
        t0.fBackend != t1.fBackend) {
        return false;
    }

    switch (t0.fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kMock:     [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return t0.fTextureData->equal(t1.fTextureData.get());
        default:
            return false;
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

GrBackendRenderTarget::GrBackendRenderTarget() : fIsValid(false) {}

GrBackendRenderTarget::~GrBackendRenderTarget() = default;

GrBackendRenderTarget::GrBackendRenderTarget(const GrBackendRenderTarget& that) : fIsValid(false) {
    *this = that;
}

GrBackendRenderTarget& GrBackendRenderTarget::operator=(const GrBackendRenderTarget& that) {
    if (this == &that) {
        return *this;
    }

    fRTData.reset();
    if (!that.isValid()) {
        fIsValid = false;
        return *this;
    }
    fWidth = that.fWidth;
    fHeight = that.fHeight;
    fSampleCnt = that.fSampleCnt;
    fStencilBits = that.fStencilBits;
    fBackend = that.fBackend;

    switch (that.fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kMock:     [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            that.fRTData->copyTo(fRTData);
            break;
        default:
            SK_ABORT("Unknown GrBackend");
    }
    fIsValid = true;
    return *this;
}

sk_sp<skgpu::MutableTextureState> GrBackendRenderTarget::getMutableState() const {
    return fRTData->getMutableState();
}

GrBackendFormat GrBackendRenderTarget::getBackendFormat() const {
    if (!this->isValid()) {
        return GrBackendFormat();
    }
    switch (fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kMock:     [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return fRTData->getBackendFormat();
        default:
            return GrBackendFormat();
    }
}

void GrBackendRenderTarget::setMutableState(const skgpu::MutableTextureState& state) {
    fRTData->setMutableState(state);
}

bool GrBackendRenderTarget::isProtected() const {
    if (!this->isValid()) {
        return false;
    }
    return fRTData->isProtected();
}

#if defined(GPU_TEST_UTILS)
bool GrBackendRenderTarget::TestingOnly_Equals(const GrBackendRenderTarget& r0,
                                               const GrBackendRenderTarget& r1) {
    if (!r0.isValid() || !r1.isValid()) {
        return false; // two invalid backend rendertargets are not considered equal
    }

    if (r0.fWidth != r1.fWidth ||
        r0.fHeight != r1.fHeight ||
        r0.fSampleCnt != r1.fSampleCnt ||
        r0.fStencilBits != r1.fStencilBits ||
        r0.fBackend != r1.fBackend) {
        return false;
    }

    switch (r0.fBackend) {
        case GrBackendApi::kDirect3D: [[fallthrough]];
        case GrBackendApi::kMetal:    [[fallthrough]];
        case GrBackendApi::kMock:     [[fallthrough]];
        case GrBackendApi::kOpenGL:   [[fallthrough]];
        case GrBackendApi::kVulkan:
            return r0.fRTData->equal(r1.fRTData.get());
        default:
            return false;
    }

    SkASSERT(0);
    return false;
}
#endif

GrBackendFormatData::~GrBackendFormatData() {}
GrBackendTextureData::~GrBackendTextureData() {}
GrBackendRenderTargetData::~GrBackendRenderTargetData() {}
