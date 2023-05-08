/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkGraphics.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkOpenTypeSVGDecoder.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/core/SkTime.h"
#include "include/private/base/SkMath.h"
#include "src/base/SkTSearch.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkCpu.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkOpts.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkTypefaceCache.h"

#include <stdlib.h>

#include "include/codec/SkBmpDecoder.h"
#include "include/codec/SkWbmpDecoder.h"

#ifdef SK_HAS_WUFFS_LIBRARY
#include "include/codec/SkGifDecoder.h"
#endif

#ifdef SK_CODEC_DECODES_JPEG
#include "include/codec/SkJpegDecoder.h"
#endif

#ifdef SK_CODEC_DECODES_JPEGXL
#include "include/codec/SkJpegxlDecoder.h"
#endif

#ifdef SK_CODEC_DECODES_PNG
#include "include/codec/SkIcoDecoder.h"
#include "include/codec/SkPngDecoder.h"
#endif

#ifdef SK_CODEC_DECODES_RAW
#include "include/codec/SkRawDecoder.h"
#endif

#ifdef SK_CODEC_DECODES_WEBP
#include "include/codec/SkWebpDecoder.h"
#endif

static void legacy_init_decoders() {
#ifdef SK_CODEC_DECODES_JPEG
    SkCodecs::Register(SkJpegDecoder::Decoder());
#endif
#ifdef SK_CODEC_DECODES_WEBP
    SkCodecs::Register(SkWebpDecoder::Decoder());
#endif
#ifdef SK_HAS_WUFFS_LIBRARY
    SkCodecs::Register(SkGifDecoder::Decoder());
#endif
#ifdef SK_CODEC_DECODES_PNG
    SkCodecs::Register(SkPngDecoder::Decoder());
    SkCodecs::Register(SkIcoDecoder::Decoder());
#endif
    SkCodecs::Register(SkBmpDecoder::Decoder());
    SkCodecs::Register(SkWbmpDecoder::Decoder());
    // FIXME
    // #ifdef SK_CODEC_DECODES_AVIF
    //     { SkAvifCodec::IsAvif, SkAvifCodec::MakeFromStream },
    // #endif
#ifdef SK_CODEC_DECODES_JPEGXL
    SkCodecs::Register(SkJpegxlDecoder::Decoder());
#endif
#ifdef SK_CODEC_DECODES_RAW
    SkCodecs::Register(SkRawDecoder::Decoder());
#endif
}


void SkGraphics::Init() {
    // SkGraphics::Init() must be thread-safe and idempotent.
    SkCpu::CacheRuntimeFeatures();
    SkOpts::Init();
    legacy_init_decoders();
}

///////////////////////////////////////////////////////////////////////////////

void SkGraphics::DumpMemoryStatistics(SkTraceMemoryDump* dump) {
  SkResourceCache::DumpMemoryStatistics(dump);
  SkStrikeCache::DumpMemoryStatistics(dump);
}

void SkGraphics::PurgeAllCaches() {
    SkGraphics::PurgeFontCache();
    SkGraphics::PurgeResourceCache();
    SkImageFilter_Base::PurgeCache();
}

///////////////////////////////////////////////////////////////////////////////

size_t SkGraphics::GetFontCacheLimit() {
    return SkStrikeCache::GlobalStrikeCache()->getCacheSizeLimit();
}

size_t SkGraphics::SetFontCacheLimit(size_t bytes) {
    return SkStrikeCache::GlobalStrikeCache()->setCacheSizeLimit(bytes);
}

size_t SkGraphics::GetFontCacheUsed() {
    return SkStrikeCache::GlobalStrikeCache()->getTotalMemoryUsed();
}

int SkGraphics::GetFontCacheCountLimit() {
    return SkStrikeCache::GlobalStrikeCache()->getCacheCountLimit();
}

int SkGraphics::SetFontCacheCountLimit(int count) {
    return SkStrikeCache::GlobalStrikeCache()->setCacheCountLimit(count);
}

int SkGraphics::GetFontCacheCountUsed() {
    return SkStrikeCache::GlobalStrikeCache()->getCacheCountUsed();
}

void SkGraphics::PurgeFontCache() {
    SkStrikeCache::GlobalStrikeCache()->purgeAll();
    SkTypefaceCache::PurgeAll();
}

static SkGraphics::OpenTypeSVGDecoderFactory gSVGDecoderFactory = nullptr;

SkGraphics::OpenTypeSVGDecoderFactory
SkGraphics::SetOpenTypeSVGDecoderFactory(OpenTypeSVGDecoderFactory svgDecoderFactory) {
    OpenTypeSVGDecoderFactory old(gSVGDecoderFactory);
    gSVGDecoderFactory = svgDecoderFactory;
    return old;
}

SkGraphics::OpenTypeSVGDecoderFactory SkGraphics::GetOpenTypeSVGDecoderFactory() {
    return gSVGDecoderFactory;
}

extern bool gSkVMAllowJIT;

void SkGraphics::AllowJIT() {
    gSkVMAllowJIT = true;
}
