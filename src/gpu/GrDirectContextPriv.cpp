/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDirectContextPriv.h"

#include "include/gpu/GrContextThreadSafeProxy.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrThreadSafePipelineBuilder.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/SurfaceContext.h"
#include "src/gpu/SurfaceFillContext.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/text/GrAtlasManager.h"
#include "src/gpu/text/GrTextBlobCache.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Gpu.h"

#define ASSERT_OWNED_PROXY(P) \
    SkASSERT(!(P) || !((P)->peekTexture()) || (P)->peekTexture()->getContext() == this->context())
#define ASSERT_SINGLE_OWNER GR_ASSERT_SINGLE_OWNER(this->context()->singleOwner())
#define RETURN_VALUE_IF_ABANDONED(value) if (this->context()->abandoned()) { return (value); }

GrSemaphoresSubmitted GrDirectContextPriv::flushSurfaces(
                                                    SkSpan<GrSurfaceProxy*> proxies,
                                                    SkSurface::BackendSurfaceAccess access,
                                                    const GrFlushInfo& info,
                                                    const GrBackendSurfaceMutableState* newState) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("GrDirectContextPriv", "flushSurfaces", this->context());

    if (this->context()->abandoned()) {
        if (info.fSubmittedProc) {
            info.fSubmittedProc(info.fSubmittedContext, false);
        }
        if (info.fFinishedProc) {
            info.fFinishedProc(info.fFinishedContext);
        }
        return GrSemaphoresSubmitted::kNo;
    }

#ifdef SK_DEBUG
    for (GrSurfaceProxy* proxy : proxies) {
        SkASSERT(proxy);
        ASSERT_OWNED_PROXY(proxy);
    }
#endif
    return this->context()->drawingManager()->flushSurfaces(proxies, access, info, newState);
}

void GrDirectContextPriv::createDDLTask(sk_sp<const SkDeferredDisplayList> ddl,
                                        sk_sp<GrRenderTargetProxy> newDest,
                                        SkIPoint offset) {
    this->context()->drawingManager()->createDDLTask(std::move(ddl), std::move(newDest), offset);
}

bool GrDirectContextPriv::compile(const GrProgramDesc& desc, const GrProgramInfo& info) {
    GrGpu* gpu = this->getGpu();
    if (!gpu) {
        return false;
    }

    return gpu->compile(desc, info);
}


//////////////////////////////////////////////////////////////////////////////
#if GR_TEST_UTILS

void GrDirectContextPriv::dumpCacheStats(SkString* out) const {
#if GR_CACHE_STATS
    this->context()->fResourceCache->dumpStats(out);
#endif
}

void GrDirectContextPriv::dumpCacheStatsKeyValuePairs(SkTArray<SkString>* keys,
                                                      SkTArray<double>* values) const {
#if GR_CACHE_STATS
    this->context()->fResourceCache->dumpStatsKeyValuePairs(keys, values);
#endif
}

void GrDirectContextPriv::printCacheStats() const {
    SkString out;
    this->dumpCacheStats(&out);
    SkDebugf("%s", out.c_str());
}

/////////////////////////////////////////////////
void GrDirectContextPriv::resetGpuStats() const {
#if GR_GPU_STATS
    this->context()->fGpu->stats()->reset();
#endif
}

void GrDirectContextPriv::dumpGpuStats(SkString* out) const {
#if GR_GPU_STATS
    this->context()->fGpu->stats()->dump(out);
    if (auto builder = this->context()->fGpu->pipelineBuilder()) {
        builder->stats()->dump(out);
    }
#endif
}

void GrDirectContextPriv::dumpGpuStatsKeyValuePairs(SkTArray<SkString>* keys,
                                                    SkTArray<double>* values) const {
#if GR_GPU_STATS
    this->context()->fGpu->stats()->dumpKeyValuePairs(keys, values);
    if (auto builder = this->context()->fGpu->pipelineBuilder()) {
        builder->stats()->dumpKeyValuePairs(keys, values);
    }
#endif
}

void GrDirectContextPriv::printGpuStats() const {
    SkString out;
    this->dumpGpuStats(&out);
    SkDebugf("%s", out.c_str());
}

/////////////////////////////////////////////////
void GrDirectContextPriv::resetContextStats() {
#if GR_GPU_STATS
    this->context()->stats()->reset();
#endif
}

void GrDirectContextPriv::dumpContextStats(SkString* out) const {
#if GR_GPU_STATS
    this->context()->stats()->dump(out);
#endif
}

void GrDirectContextPriv::dumpContextStatsKeyValuePairs(SkTArray<SkString>* keys,
                                                        SkTArray<double>* values) const {
#if GR_GPU_STATS
    this->context()->stats()->dumpKeyValuePairs(keys, values);
#endif
}

void GrDirectContextPriv::printContextStats() const {
    SkString out;
    this->dumpContextStats(&out);
    SkDebugf("%s", out.c_str());
}

/////////////////////////////////////////////////
sk_sp<SkImage> GrDirectContextPriv::testingOnly_getFontAtlasImage(GrMaskFormat format,
                                                                  unsigned int index) {
    auto atlasManager = this->getAtlasManager();
    if (!atlasManager) {
        return nullptr;
    }

    unsigned int numActiveProxies;
    const GrSurfaceProxyView* views = atlasManager->getViews(format, &numActiveProxies);
    if (index >= numActiveProxies || !views || !views[index].proxy()) {
        return nullptr;
    }

    SkColorType colorType = GrColorTypeToSkColorType(GrMaskFormatToColorType(format));
    SkASSERT(views[index].proxy()->priv().isExact());
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(this->context()),
                                   kNeedNewImageUniqueID,
                                   views[index],
                                   SkColorInfo(colorType, kPremul_SkAlphaType, nullptr));
}

void GrDirectContextPriv::testingOnly_flushAndRemoveOnFlushCallbackObject(
        GrOnFlushCallbackObject* cb) {
    this->context()->flushAndSubmit();
    this->context()->drawingManager()->testingOnly_removeOnFlushCallbackObject(cb);
}
#endif

// Both of these effects aggressively round to the nearest exact (N / 255) floating point values.
// This lets us find a round-trip preserving pair on some GPUs that do odd byte to float conversion.
static std::unique_ptr<GrFragmentProcessor> make_premul_effect(
        std::unique_ptr<GrFragmentProcessor> fp) {
    if (!fp) {
        return nullptr;
    }

    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter, R"(
        half4 main(half4 color) {
            color = floor(color * 255 + 0.5) / 255;
            color.rgb = floor(color.rgb * color.a * 255 + 0.5) / 255;
            return color;
        }
    )");

    fp = GrSkSLFP::Make(effect, "ToPremul", std::move(fp), GrSkSLFP::OptFlags::kNone);
    return GrFragmentProcessor::HighPrecision(std::move(fp));
}

static std::unique_ptr<GrFragmentProcessor> make_unpremul_effect(
        std::unique_ptr<GrFragmentProcessor> fp) {
    if (!fp) {
        return nullptr;
    }

    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter, R"(
        half4 main(half4 color) {
            color = floor(color * 255 + 0.5) / 255;
            color.rgb = color.a <= 0 ? half3(0) : floor(color.rgb / color.a * 255 + 0.5) / 255;
            return color;
        }
    )");

    fp = GrSkSLFP::Make(effect, "ToUnpremul", std::move(fp), GrSkSLFP::OptFlags::kNone);
    return GrFragmentProcessor::HighPrecision(std::move(fp));
}

static bool test_for_preserving_PM_conversions(GrDirectContext*) {
    return false;
}

bool GrDirectContextPriv::validPMUPMConversionExists() {
    ASSERT_SINGLE_OWNER

    auto dContext = this->context();

    if (!dContext->fDidTestPMConversions) {
        dContext->fPMUPMConversionsRoundTrip = test_for_preserving_PM_conversions(dContext);
        dContext->fDidTestPMConversions = true;
    }

    // The PM<->UPM tests fail or succeed together so we only need to check one.
    return dContext->fPMUPMConversionsRoundTrip;
}

std::unique_ptr<GrFragmentProcessor> GrDirectContextPriv::createPMToUPMEffect(
        std::unique_ptr<GrFragmentProcessor> fp) {
    ASSERT_SINGLE_OWNER
    // We should have already called this->priv().validPMUPMConversionExists() in this case
    SkASSERT(this->context()->fDidTestPMConversions);
    // ...and it should have succeeded
    SkASSERT(this->validPMUPMConversionExists());

    return make_unpremul_effect(std::move(fp));
}

std::unique_ptr<GrFragmentProcessor> GrDirectContextPriv::createUPMToPMEffect(
        std::unique_ptr<GrFragmentProcessor> fp) {
    ASSERT_SINGLE_OWNER
    // We should have already called this->priv().validPMUPMConversionExists() in this case
    SkASSERT(this->context()->fDidTestPMConversions);
    // ...and it should have succeeded
    SkASSERT(this->validPMUPMConversionExists());

    return make_premul_effect(std::move(fp));
}
