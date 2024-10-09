/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef DisplayParams_DEFINED
#define DisplayParams_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/ganesh/GrContextOptions.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/ContextOptions.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "tools/graphite/TestOptions.h"
#endif

namespace skwindow {

#if defined(SK_GRAPHITE)
struct GraphiteTestOptions {
    GraphiteTestOptions() {
        fTestOptions.fContextOptions.fOptionsPriv = &fPriv;
    }

    GraphiteTestOptions(const GraphiteTestOptions& other)
            : fTestOptions(other.fTestOptions)
            , fPriv(other.fPriv) {
        fTestOptions.fContextOptions.fOptionsPriv = &fPriv;
    }

    GraphiteTestOptions& operator=(const GraphiteTestOptions& other) {
        fTestOptions = other.fTestOptions;
        fPriv = other.fPriv;
        fTestOptions.fContextOptions.fOptionsPriv = &fPriv;
        return *this;
    }

    skiatest::graphite::TestOptions     fTestOptions;
    skgpu::graphite::ContextOptionsPriv fPriv;
};

#endif

// DisplayParams should be treated as a immutable object once created.
class DisplayParams {
public:
    DisplayParams()
        : fColorType(kN32_SkColorType)
        , fColorSpace(nullptr)
        , fMSAASampleCount(1)
        , fSurfaceProps(0, kRGB_H_SkPixelGeometry)
        , fDisableVsync(false)
        , fDelayDrawableAcquisition(false)
        , fCreateProtectedNativeBackend(false)
    {}

    class Builder {
    public:
        DisplayParamsBuilder() {}

        DisplayParamsBuilder(const DisplayParamsBuilder& other)
                : fDisplayParams(other) {}

        DisplayParamsBuilder& operator=(const DisplayParamsBuilder& other) = delete;

        DisplayParamsBuilder& colorType(SkColorType colorType) {
            fDisplayParams->fColorType = colorType;
            return *this;
        }

        DisplayParamsBuilder& colorSpace(const sk_sp<SkColorSpace>& colorSpace) {
            fDisplayParams->fColorSpace = colorSpace;
            return *this;
        }

        DisplayParamsBuilder& MSAASampleCount(int MSAASampleCount) {
            fDisplayParams->fMSAASampleCount = MSAASampleCount;
            return *this;
        }

        DisplayParamsBuilder& grContextOptions(const GrContextOptions& grContextOptions) {
            fDisplayParams->fGrContextOptions = grContextOptions;
            return *this;
        }

#if defined(SK_GRAPHITE)
        DisplayParamsBuilder& graphiteTestOptions(const GraphiteTestOptions& graphiteTestOptions) {
            fDisplayParams->fGraphiteTestOptions = graphiteTestOptions;
            return *this;
        }
#endif

        DisplayParamsBuilder& surfaceProps(const SkSurfaceProps& surfaceProps) {
            fDisplayParams->fSurfaceProps = surfaceProps;
            return *this;
        }

        DisplayParamsBuilder& disableVsync(bool disableVsync) {
            fDisplayParams->fDisableVsync = disableVsync;
            return *this;
        }

        DisplayParamsBuilder& delayDrawableAcquisition(bool delayDrawableAcquisition) {
            fDisplayParams->fDelayDrawableAcquisition = delayDrawableAcquisition;
            return *this;
        }

        DisplayParamsBuilder& createProtectedNativeBackend(bool createProtectedNativeBackend) {
            fDisplayParams->fCreateProtectedNativeBackend = createProtectedNativeBackend;
            return *this;
        }

        sk_sp<DisplayParams> build() { return fDisplayParams; }

    private:
        DisplayParams fDisplayParams;
    };

private:
    friend class DisplayParams::Builder;

    SkColorType            fColorType;
    sk_sp<SkColorSpace>    fColorSpace;
    int                    fMSAASampleCount;
    GrContextOptions       fGrContextOptions;
#if defined(SK_GRAPHITE)
    GraphiteTestOptions    fGraphiteTestOptions;
#endif
    SkSurfaceProps         fSurfaceProps;
    bool                   fDisableVsync;
    bool                   fDelayDrawableAcquisition;
    bool                   fCreateProtectedNativeBackend = false;
};

}  // namespace skwindow

#endif
