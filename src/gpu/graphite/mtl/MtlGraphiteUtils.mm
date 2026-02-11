/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/mtl/MtlGraphiteUtils.h"
#include "src/gpu/graphite/mtl/MtlQueueManager.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {

namespace ContextFactory {

std::unique_ptr<Context> MakeMetal(const MtlBackendContext& backendContext,
                                   const ContextOptions& options) {
    sk_sp<SharedContext> sharedContext = MtlSharedContext::Make(backendContext, options);
    if (!sharedContext) {
        return nullptr;
    }

    sk_cfp<id<MTLCommandQueue>> queue =
            sk_ret_cfp((id<MTLCommandQueue>)(backendContext.fQueue.get()));
    auto queueManager = std::make_unique<MtlQueueManager>(std::move(queue), sharedContext.get());
    if (!queueManager) {
        return nullptr;
    }

    return ContextCtorAccessor::MakeContext(std::move(sharedContext),
                                            std::move(queueManager),
                                            options);
}

} // namespace ContextFactory

sk_cfp<id<MTLLibrary>> MtlCompileShaderLibrary(const MtlSharedContext* sharedContext,
                                               std::string_view label,
                                               std::string_view msl,
                                               ShaderErrorHandler* errorHandler) {
    TRACE_EVENT0("skia.shaders", "driver_compile_shader");
    NSString* nsSource = [[NSString alloc] initWithBytesNoCopy:const_cast<char*>(msl.data())
                                                        length:msl.size()
                                                      encoding:NSUTF8StringEncoding
                                                  freeWhenDone:NO];
    if (!nsSource) {
        return nil;
    }
    MTLCompileOptions* options = [[MTLCompileOptions alloc] init];

    // Framebuffer fetch is supported in MSL 2.3 in MacOS 11+.
    if (@available(macOS 11.0, iOS 14.0, tvOS 14.0, *)) {
        options.languageVersion = MTLLanguageVersion2_3;

    // array<> is supported in MSL 2.0 on MacOS 10.13+ and iOS 11+,
    // and in MSL 1.2 on iOS 10+ (but not MacOS).
    } else if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
        options.languageVersion = MTLLanguageVersion2_0;
#if defined(SK_BUILD_FOR_IOS)
    } else if (@available(macOS 10.12, iOS 10.0, tvOS 10.0, *)) {
        options.languageVersion = MTLLanguageVersion1_2;
#endif
    }

    NSError* error = nil;
    // TODO: do we need a version with a timeout?
    sk_cfp<id<MTLLibrary>> compiledLibrary(
            [sharedContext->device() newLibraryWithSource:(NSString* _Nonnull)nsSource
                                                  options:options
                                                    error:&error]);
    if (!compiledLibrary) {
        std::string mslStr(msl);
        errorHandler->compileError(
                mslStr.c_str(), error.debugDescription.UTF8String, /*shaderWasCached=*/false);
        return nil;
    }

    NSString* nsLabel = [[NSString alloc] initWithBytesNoCopy:const_cast<char*>(label.data())
                                                       length:label.size()
                                                     encoding:NSUTF8StringEncoding
                                                 freeWhenDone:NO];
    compiledLibrary.get().label = nsLabel;
    return compiledLibrary;
}

namespace {

// Most formats in MTL_FORMAT_MAPPING for TextureFormat are always available for Graphite's
// supported Mac and iOS versions. These formats are not. To simplify the mapping, these constants
// are extracted from Frameworks/Metal/MTLPixelFormat.h to avoid guarding with an @available check,
// although MtlCaps must still do so when enabling a particular format.

#define MTL_PIXEL_FORMAT(name, value) \
    static constexpr MTLPixelFormat name = static_cast<MTLPixelFormat>(value)

// Either unsupported on iOS or high min iOS, so considered "mac-only"
MTL_PIXEL_FORMAT(MTLPixelFormatBC1_RGBA_, 130);
MTL_PIXEL_FORMAT(MTLPixelFormatBC1_RGBA_sRGB_, 131);
MTL_PIXEL_FORMAT(MTLPixelFormatDepth24Unorm_Stencil8_, 255);
MTL_PIXEL_FORMAT(MTLPixelFormatX24_Stencil8_, 262);
// Requires Mac 11.0+, while we support 10.15+. All meet Graphite's minimum supported iOS version
MTL_PIXEL_FORMAT(MTLPixelFormatB5G6R5Unorm_, 40);
MTL_PIXEL_FORMAT(MTLPixelFormatA1BGR5Unorm_, 41);
MTL_PIXEL_FORMAT(MTLPixelFormatABGR4Unorm_, 42);
MTL_PIXEL_FORMAT(MTLPixelFormatBGR5A1Unorm_, 43);
MTL_PIXEL_FORMAT(MTLPixelFormatBGR10_XR_sRGB_, 555);
MTL_PIXEL_FORMAT(MTLPixelFormatBGR10_XR_, 554);
MTL_PIXEL_FORMAT(MTLPixelFormatBGRA10_XR_sRGB_, 553);
MTL_PIXEL_FORMAT(MTLPixelFormatBGRA10_XR_, 552);
MTL_PIXEL_FORMAT(MTLPixelFormatETC2_RGB8_, 180);
MTL_PIXEL_FORMAT(MTLPixelFormatETC2_RGB8_sRGB_, 181);

#if defined(SK_BUILD_FOR_MAC)
static_assert(MTLPixelFormatBC1_RGBA_ == MTLPixelFormatBC1_RGBA);
static_assert(MTLPixelFormatBC1_RGBA_sRGB_ == MTLPixelFormatBC1_RGBA_sRGB);
static_assert(MTLPixelFormatDepth24Unorm_Stencil8_ == MTLPixelFormatDepth24Unorm_Stencil8);
static_assert(MTLPixelFormatX24_Stencil8_ == MTLPixelFormatX24_Stencil8);
#endif

static void validate_mtl_pixelformats() {
    if (@available(macOS 11.0, iOS 10.0, *)) {
        SkASSERT(MTLPixelFormatB5G6R5Unorm_ == MTLPixelFormatB5G6R5Unorm);
        SkASSERT(MTLPixelFormatA1BGR5Unorm_ == MTLPixelFormatA1BGR5Unorm);
        SkASSERT(MTLPixelFormatABGR4Unorm_ == MTLPixelFormatABGR4Unorm);
        SkASSERT(MTLPixelFormatBGR5A1Unorm_ == MTLPixelFormatBGR5A1Unorm);
        SkASSERT(MTLPixelFormatBGR10_XR_ == MTLPixelFormatBGR10_XR);
        SkASSERT(MTLPixelFormatBGR10_XR_sRGB_ == MTLPixelFormatBGR10_XR_sRGB);
        SkASSERT(MTLPixelFormatBGRA10_XR_ == MTLPixelFormatBGRA10_XR);
        SkASSERT(MTLPixelFormatBGRA10_XR_sRGB_ == MTLPixelFormatBGRA10_XR_sRGB);
        SkASSERT(MTLPixelFormatETC2_RGB8_ == MTLPixelFormatETC2_RGB8);
        SkASSERT(MTLPixelFormatETC2_RGB8_sRGB_ == MTLPixelFormatETC2_RGB8_sRGB);
    }
}

struct FamilyFeatureMap {
    using FormatFeatureSet = std::pair<MTLPixelFormat, /*FeatureFlag mask*/uint8_t>;
    using BulkFeature = std::pair</*condition mask*/uint8_t, /*newFeatureFlags mask*/uint8_t>;

    FamilyFeatureMap(std::initializer_list<FormatFeatureSet> features)
            : FamilyFeatureMap(/*parent=*/nullptr, features, /*bulkExtension=*/{0, 0}) {}

    FamilyFeatureMap(const FamilyFeatureMap* parent,
                     std::initializer_list<FormatFeatureSet> extensions,
                     BulkFeature bulkExtension) {
        for (int i = 0; i < kTextureFormatCount; ++i) {
            fFormatFeatures[i] = parent ? parent->fFormatFeatures[i] : 0;
            // Apply bulk extension based on parent features, not any specific new extensions
            if ((fFormatFeatures[i] & bulkExtension.first) == bulkExtension.first) {
                fFormatFeatures[i] |= bulkExtension.second;
            }
        }
        for (const FormatFeatureSet& set : extensions) {
            TextureFormat tf = MTLPixelFormatToTextureFormat(set.first);
            if (tf == TextureFormat::kUnsupported) { continue; } // Skip if Graphite doesn't use it

            fFormatFeatures[(int) tf] |= set.second;
        }
        SkASSERT(fFormatFeatures[(int) TextureFormat::kUnsupported] == 0);
    }

    // We only care about MTLPixelFormats that map back to a TextureFormat.
    std::array<uint8_t, kTextureFormatCount> fFormatFeatures;
};

FamilyFeatureMap extend(const FamilyFeatureMap& parent,
                        std::initializer_list<FamilyFeatureMap::FormatFeatureSet> extensions,
                        FamilyFeatureMap::BulkFeature bulkExtension = {0, 0}) {
    return FamilyFeatureMap(&parent, extensions, bulkExtension);
}

// Reproduction of "Texture capabalities by pixel format" from
//     https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
// NOTE: All graphics and compute kernels can read or sample a texture with any pixel format.
enum FeatureFlag : uint8_t {
    NotAvailable = 0x0,  // Not available.
    Atomic       = 0x1,  // The GPU can use atomic operations on textures with the pixel format.
    Filter       = 0x2,  // The GPU can filter a texture with the pixel format during sampling.
    Write        = 0x4,  // The GPU can write to a texture on a per-pixel basis with the format.
    Color        = 0x8,  // The GPU can use a texture with the format as a color render target.
    Blend        = 0x10, // The GPU can blend a texture with the pixel format.
    MSAA         = 0x20, // The GPU can use a texture with the pixel format as a destination for
                         // multisample antialias (MSAA) data.
    Sparse       = 0x40, // The GPU supports sparse-texture allocations for textures in this format.
    Resolve      = 0x80, // The GPU can use a texture with the pixel format as a source for
                         // multisample antialias (MSAA) resolve operations.

    // Sparse isn’t included in All since the Mac2, Metal3, and Apple2 through Apple5 family GPUs,
    // don’t support the sparse texture feature. Apple6+ references to All in the table implicitly
    // include Sparse.
    // ERRATA: The PDF claims Apple6 does not support Sparse, but the column for Apple6 adds Sparse
    // to rows that weren't already listed as "All", and
    // https://developer.apple.com/documentation/metal/managing-sparse-texture-memory
    // suggests Apple6 is when sparse texture support was added.
    All = Filter | Write | Color | Blend | MSAA | Resolve,
};

static const FamilyFeatureMap kApple2 = {
    // Ordinary 8-bit pixel foramts
    {MTLPixelFormatA8Unorm,                Filter},
    {MTLPixelFormatR8Unorm,                All},
    {MTLPixelFormatR8Unorm_sRGB,           All},
    {MTLPixelFormatR8Snorm,                All},
    {MTLPixelFormatR8Uint,                 Write | Color | MSAA},
    {MTLPixelFormatR8Sint,                 Write | Color | MSAA},
    // Ordinary 16-bit pixel formats
    {MTLPixelFormatR16Unorm,               Filter | Write | Color | MSAA | Blend},
    {MTLPixelFormatR16Snorm,               Filter | Write | Color | MSAA | Blend},
    {MTLPixelFormatR16Uint,                Write | Color | MSAA},
    {MTLPixelFormatR16Sint,                Write | Color | MSAA},
    {MTLPixelFormatR16Float,               All},
    {MTLPixelFormatRG8Unorm,               All},
    {MTLPixelFormatRG8Unorm_sRGB,          All},
    {MTLPixelFormatRG8Snorm,               All},
    {MTLPixelFormatRG8Uint,                Write | Color | MSAA},
    {MTLPixelFormatRG8Sint,                Write | Color | MSAA},
    // Packed 16-bit pixel formats
    {MTLPixelFormatB5G6R5Unorm_,           Filter | Color | MSAA | Resolve | Blend},
    {MTLPixelFormatA1BGR5Unorm_,           Filter | Color | MSAA | Resolve | Blend},
    {MTLPixelFormatABGR4Unorm_,            Filter | Color | MSAA | Resolve | Blend},
    {MTLPixelFormatBGR5A1Unorm_,           Filter | Color | MSAA | Resolve | Blend},
    // Ordinary 32-bit pixel formats
    {MTLPixelFormatR32Uint,                Write | Color},
    {MTLPixelFormatR32Sint,                Write | Color},
    {MTLPixelFormatR32Float,               Write | Color | MSAA | Blend},
    {MTLPixelFormatRG16Unorm,              Filter | Write | Color | MSAA | Blend},
    {MTLPixelFormatRG16Snorm,              Filter | Write | Color | MSAA | Blend},
    {MTLPixelFormatRG16Uint,               Write | Color | MSAA},
    {MTLPixelFormatRG16Sint,               Write | Color | MSAA},
    {MTLPixelFormatRG16Float,              All},
    {MTLPixelFormatRGBA8Unorm,             All},
    {MTLPixelFormatRGBA8Unorm_sRGB,        All},
    {MTLPixelFormatRGBA8Snorm,             All},
    {MTLPixelFormatRGBA8Uint,              Write | Color | MSAA},
    {MTLPixelFormatRGBA8Sint,              Write | Color | MSAA},
    {MTLPixelFormatBGRA8Unorm,             All},
    {MTLPixelFormatBGRA8Unorm_sRGB,        All},
    // Packed 32-bit pixel formats
    {MTLPixelFormatRGB10A2Unorm,           Filter | Color | MSAA | Resolve | Blend},
    {MTLPixelFormatBGR10A2Unorm,           All},
    {MTLPixelFormatRGB10A2Uint,            Color | MSAA},
    {MTLPixelFormatRG11B10Float,           Filter | Color | MSAA | Resolve | Blend},
    {MTLPixelFormatRGB9E5Float,            Filter | Color | MSAA | Resolve | Blend},
    // Ordinary 64-bit pixel formats
    {MTLPixelFormatRG32Uint,               Write | Color},
    {MTLPixelFormatRG32Sint,               Write | Color},
    {MTLPixelFormatRG32Float,              Write | Color | Blend},
    {MTLPixelFormatRGBA16Unorm,            Filter | Write | Color | MSAA | Blend},
    {MTLPixelFormatRGBA16Snorm,            Filter | Write | Color | MSAA | Blend},
    {MTLPixelFormatRGBA16Uint,             Write | Color | MSAA},
    {MTLPixelFormatRGBA16Sint,             Write | Color | MSAA},
    {MTLPixelFormatRGBA16Float,            All},
    // Ordinary 128-bit pixel formats
    {MTLPixelFormatRGBA32Uint,             Write | Color},
    {MTLPixelFormatRGBA32Sint,             Write | Color},
    {MTLPixelFormatRGBA32Float,            Write | Color | Blend},
    // Compressed pixel formats (only compression types supported by Skia)
    {MTLPixelFormatETC2_RGB8_,             Filter},
    {MTLPixelFormatETC2_RGB8_sRGB_,        Filter},
    {MTLPixelFormatBC1_RGBA_,              NotAvailable},
    {MTLPixelFormatBC1_RGBA_sRGB_,         NotAvailable},
    // YUV pixel foramts
    {MTLPixelFormatGBGR422,                Filter},
    {MTLPixelFormatBGRG422,                Filter},
    // Depth and stencil pixel formats
    {MTLPixelFormatDepth16Unorm,           Filter | MSAA},
    {MTLPixelFormatDepth32Float,           MSAA},
    {MTLPixelFormatStencil8,               MSAA},
    {MTLPixelFormatDepth24Unorm_Stencil8_, NotAvailable},
    {MTLPixelFormatDepth32Float_Stencil8,  MSAA},
    {MTLPixelFormatX24_Stencil8_,          NotAvailable},
    {MTLPixelFormatX32_Stencil8,           NotAvailable},
    // Extended-range and wide-color pixel formats
    {MTLPixelFormatBGRA10_XR_,             NotAvailable},
    {MTLPixelFormatBGRA10_XR_sRGB_,        NotAvailable},
    {MTLPixelFormatBGR10_XR_,              NotAvailable},
    {MTLPixelFormatBGR10_XR_sRGB_,         NotAvailable}
};

static const FamilyFeatureMap kApple3 = extend(kApple2, {
    // Ordinary 8-bit pixel formats
    {MTLPixelFormatA8Unorm,                All},
    // Packed 32-bit pixel formats
    {MTLPixelFormatRGB10A2Unorm,           Write},
    {MTLPixelFormatRGB10A2Uint,            Write},
    {MTLPixelFormatRG11B10Float,           Write},
    {MTLPixelFormatRGB9E5Float,            Write},
    // Depth and stencil pixel formats,
    {MTLPixelFormatDepth16Unorm,           Resolve},
    {MTLPixelFormatDepth32Float,           Resolve},
    {MTLPixelFormatStencil8,               Resolve},
    {MTLPixelFormatDepth32Float_Stencil8,  Resolve},
    // Extended-range and wide-color pixel formats
    {MTLPixelFormatBGRA10_XR_,             All},
    {MTLPixelFormatBGRA10_XR_sRGB_,        All},
    {MTLPixelFormatBGR10_XR_,              All},
    {MTLPixelFormatBGR10_XR_sRGB_,         All}
});

static const FamilyFeatureMap kApple4 = extend(kApple3, {
    // Ordinary 16-bit pixel formats
    {MTLPixelFormatR16Unorm,               Resolve},
    {MTLPixelFormatR16Snorm,               Resolve},
    // Ordinary 32-bit pixel formats
    {MTLPixelFormatRG16Unorm,              Resolve},
    {MTLPixelFormatRG16Snorm,              Resolve},
    // Ordinary 64-bit pixel formats
    {MTLPixelFormatRGBA16Unorm,            Resolve},
    {MTLPixelFormatRGBA16Snorm,            Resolve},
});

static const FamilyFeatureMap kApple5 = kApple4; // No changes

// Apple6 adds Sparse to every format except for depth/stencil and YUV formats, which is done in
// bulk for everything that already has the Color feature, and then manually for compressed formats.
static const FamilyFeatureMap kApple6 = extend(kApple5, {
    // Ordinary 32-bit pixel formats (these also gain Atomic in Apple6+ from having Color)
    {MTLPixelFormatR32Uint,                Atomic},
    {MTLPixelFormatR32Sint,                Atomic},
    // Compressed pixel formats (which do not include the Color feature, but do gain Sparse)
    {MTLPixelFormatETC2_RGB8_,             Sparse},
    {MTLPixelFormatETC2_RGB8_sRGB_,        Sparse}},
    // Every Color-supporting pixel format also gains Sparse in this family
    {/*condition mask=*/Color,             Sparse}
);

static const FamilyFeatureMap kApple7 = extend(kApple6, {
    // Ordinary 128-bit pixel formats
    {MTLPixelFormatRGBA32Float,            MSAA},
    // Depth and stencil formats
    {MTLPixelFormatDepth16Unorm,           Sparse},
    {MTLPixelFormatDepth32Float,           Sparse},
    {MTLPixelFormatStencil8,               Sparse},
});

static const FamilyFeatureMap kApple8 = extend(kApple7, {
    // Ordinary 64-bit pixel formats
    {MTLPixelFormatRG32Uint,               Atomic},
    {MTLPixelFormatRG32Sint,               Atomic}
});

static const FamilyFeatureMap kApple9 = extend(kApple8, {
    // Ordinary 32-bit pixel formats
    {MTLPixelFormatR32Float,               Filter | Resolve},
    // Ordinary 64-bit pixel formats
    {MTLPixelFormatRG32Float,              Filter | Resolve},
    // Ordinary 128-bit pixel formats
    {MTLPixelFormatRGBA32Float,            Filter | Resolve},
    // Compressed pixel formats
    {MTLPixelFormatBC1_RGBA_,              Filter | Sparse},
    {MTLPixelFormatBC1_RGBA_sRGB_,         Filter | Sparse},
    // Depth nd stencil pixel formats
    {MTLPixelFormatDepth32Float,           Filter},
});

// Mac2 does not extend the Apple family, it is similar but has distinct values for many formats.
// Instead of defining a shared base map that both Mac2 and Apple2 extend, these two tables are
// copied in their entirety from the spec to make comparing to the PDF easier.
static const FamilyFeatureMap kMac2 = {
    // Ordinary 8-bit pixel foramts
    {MTLPixelFormatA8Unorm,                All},
    {MTLPixelFormatR8Unorm,                All},
    {MTLPixelFormatR8Unorm_sRGB,           NotAvailable},
    {MTLPixelFormatR8Snorm,                All},
    {MTLPixelFormatR8Uint,                 Write | Color | MSAA},
    {MTLPixelFormatR8Sint,                 Write | Color | MSAA},
    // Ordinary 16-bit pixel formats
    {MTLPixelFormatR16Unorm,               All},
    {MTLPixelFormatR16Snorm,               All},
    {MTLPixelFormatR16Uint,                Write | Color | MSAA},
    {MTLPixelFormatR16Sint,                Write | Color | MSAA},
    {MTLPixelFormatR16Float,               All},
    {MTLPixelFormatRG8Unorm,               All},
    {MTLPixelFormatRG8Unorm_sRGB,          NotAvailable},
    {MTLPixelFormatRG8Snorm,               All},
    {MTLPixelFormatRG8Uint,                Write | Color | MSAA},
    {MTLPixelFormatRG8Sint,                Write | Color | MSAA},
    // Packed 16-bit pixel formats
    {MTLPixelFormatB5G6R5Unorm_,           NotAvailable},
    {MTLPixelFormatA1BGR5Unorm_,           NotAvailable},
    {MTLPixelFormatABGR4Unorm_,            NotAvailable},
    {MTLPixelFormatBGR5A1Unorm_,           NotAvailable},
    // Ordinary 32-bit pixel formats
    {MTLPixelFormatR32Uint,                Atomic | Write | Color},
    {MTLPixelFormatR32Sint,                Atomic | Write | Color},
    {MTLPixelFormatR32Float,               All},
    {MTLPixelFormatRG16Unorm,              All},
    {MTLPixelFormatRG16Snorm,              All},
    {MTLPixelFormatRG16Uint,               Write | Color | MSAA},
    {MTLPixelFormatRG16Sint,               Write | Color | MSAA},
    {MTLPixelFormatRG16Float,              All},
    {MTLPixelFormatRGBA8Unorm,             All},
    {MTLPixelFormatRGBA8Unorm_sRGB,        Filter | Color | MSAA | Resolve | Blend},
    {MTLPixelFormatRGBA8Snorm,             All},
    {MTLPixelFormatRGBA8Uint,              Write | Color | MSAA},
    {MTLPixelFormatRGBA8Sint,              Write | Color | MSAA},
    {MTLPixelFormatBGRA8Unorm,             All},
    {MTLPixelFormatBGRA8Unorm_sRGB,        Filter | Color | MSAA | Resolve | Blend},
    // Packed 32-bit pixel formats
    {MTLPixelFormatRGB10A2Unorm,           All},
    {MTLPixelFormatBGR10A2Unorm,           All},
    {MTLPixelFormatRGB10A2Uint,            Write | Color | MSAA},
    {MTLPixelFormatRG11B10Float,           All},
    {MTLPixelFormatRGB9E5Float,            Filter},
    // Ordinary 64-bit pixel formats
    {MTLPixelFormatRG32Uint,               Write | Color | MSAA},
    {MTLPixelFormatRG32Sint,               Write | Color | MSAA},
    {MTLPixelFormatRG32Float,              All},
    {MTLPixelFormatRGBA16Unorm,            All},
    {MTLPixelFormatRGBA16Snorm,            All},
    {MTLPixelFormatRGBA16Uint,             Write | Color | MSAA},
    {MTLPixelFormatRGBA16Sint,             Write | Color | MSAA},
    {MTLPixelFormatRGBA16Float,            All},
    // Ordinary 128-bit pixel formats
    {MTLPixelFormatRGBA32Uint,             Write | Color | MSAA},
    {MTLPixelFormatRGBA32Sint,             Write | Color | MSAA},
    {MTLPixelFormatRGBA32Float,            All},
    // Compressed pixel formats (only compression types supported by Skia)
    {MTLPixelFormatETC2_RGB8_,             NotAvailable},
    {MTLPixelFormatETC2_RGB8_sRGB_,        NotAvailable},
    {MTLPixelFormatBC1_RGBA_,              Filter},
    {MTLPixelFormatBC1_RGBA_sRGB_,         Filter},
    // YUV pixel foramts
    {MTLPixelFormatGBGR422,                Filter},
    {MTLPixelFormatBGRG422,                Filter},
    // Depth and stencil pixel formats
    {MTLPixelFormatDepth16Unorm,           Filter | MSAA | Resolve},
    {MTLPixelFormatDepth32Float,           Filter | MSAA | Resolve},
    {MTLPixelFormatStencil8,               NotAvailable},
    {MTLPixelFormatDepth24Unorm_Stencil8_, Filter | MSAA | Resolve},
    {MTLPixelFormatDepth32Float_Stencil8,  Filter | MSAA | Resolve},
    {MTLPixelFormatX24_Stencil8_,          MSAA},
    {MTLPixelFormatX32_Stencil8,           NotAvailable},
    // Extended-range and wide-color pixel formats
    {MTLPixelFormatBGRA10_XR_,             NotAvailable},
    {MTLPixelFormatBGRA10_XR_sRGB_,        NotAvailable},
    {MTLPixelFormatBGR10_XR_,              NotAvailable},
    {MTLPixelFormatBGR10_XR_sRGB_,         NotAvailable}
};

} // anonymous namespace

#define MTL_FORMAT_MAPPING(M) \
    M(TextureFormat::kR8,             MTLPixelFormatR8Unorm)                \
    M(TextureFormat::kR16,            MTLPixelFormatR16Unorm)               \
    M(TextureFormat::kR16F,           MTLPixelFormatR16Float)               \
    M(TextureFormat::kR32F,           MTLPixelFormatR32Float)               \
    M(TextureFormat::kA8,             MTLPixelFormatA8Unorm)                \
    M(TextureFormat::kRG8,            MTLPixelFormatRG8Unorm)               \
    M(TextureFormat::kRG16,           MTLPixelFormatRG16Unorm)              \
    M(TextureFormat::kRG16F,          MTLPixelFormatRG16Float)              \
    M(TextureFormat::kRG32F,          MTLPixelFormatRG32Float)              \
    /*TextureFormat::kRGB8,           unsupported */                        \
    /*TextureFormat::kBGR8,           unsupported */                        \
    M(TextureFormat::kB5_G6_R5,       MTLPixelFormatB5G6R5Unorm_)           \
    /*TextureFormat::kR5_G6_B5,       unsupported */                        \
    /*TextureFormat::kRGB16,          unsupported */                        \
    /*TextureFormat::kRGB16F,         unsupported */                        \
    /*TextureFormat::kRGB32F,         unsupported */                        \
    /*TextureFormat::kRGB8_sRGB,      unsupported */                        \
    M(TextureFormat::kBGR10_XR,       MTLPixelFormatBGR10_XR_)              \
    M(TextureFormat::kRGBA8,          MTLPixelFormatRGBA8Unorm)             \
    M(TextureFormat::kRGBA16,         MTLPixelFormatRGBA16Unorm)            \
    M(TextureFormat::kRGBA16F,        MTLPixelFormatRGBA16Float)            \
    M(TextureFormat::kRGBA32F,        MTLPixelFormatRGBA32Float)            \
    M(TextureFormat::kRGB10_A2,       MTLPixelFormatRGB10A2Unorm)           \
    /*TextureFormat::kRGBA10x6,       unsupported */                        \
    M(TextureFormat::kRGBA8_sRGB,     MTLPixelFormatRGBA8Unorm_sRGB)        \
    M(TextureFormat::kBGRA8,          MTLPixelFormatBGRA8Unorm)             \
    M(TextureFormat::kBGR10_A2,       MTLPixelFormatBGR10A2Unorm)           \
    M(TextureFormat::kBGRA8_sRGB,     MTLPixelFormatBGRA8Unorm_sRGB)        \
    M(TextureFormat::kABGR4,          MTLPixelFormatABGR4Unorm_)            \
    /*TextureFormat::kARGB4,          unsupported */                        \
    M(TextureFormat::kBGRA10x6_XR,    MTLPixelFormatBGRA10_XR_)             \
    M(TextureFormat::kRGB8_ETC2,      MTLPixelFormatETC2_RGB8_)             \
    M(TextureFormat::kRGB8_ETC2_sRGB, MTLPixelFormatETC2_RGB8_sRGB_)        \
    /*TextureFormat::kRGB8_BC1,       unsupported */                        \
    M(TextureFormat::kRGBA8_BC1,      MTLPixelFormatBC1_RGBA_)              \
    M(TextureFormat::kRGBA8_BC1_sRGB, MTLPixelFormatBC1_RGBA_sRGB_)         \
    /*TextureFormat::kYUV8_P2_420,    unsupported */                        \
    /*TextureFormat::kYUV8_P3_420,    unsupported */                        \
    /*TextureFormat::kYUV10x6_P2_420, unsupported */                        \
    /*TextureFormat::kExternal,       unsupported */                        \
    M(TextureFormat::kS8,             MTLPixelFormatStencil8)               \
    M(TextureFormat::kD16,            MTLPixelFormatDepth16Unorm)           \
    M(TextureFormat::kD32F,           MTLPixelFormatDepth32Float)           \
    M(TextureFormat::kD24_S8,         MTLPixelFormatDepth24Unorm_Stencil8_) \
    M(TextureFormat::kD32F_S8,        MTLPixelFormatDepth32Float_Stencil8)

TextureFormat MTLPixelFormatToTextureFormat(MTLPixelFormat format) {
#define M(TF, MTL) case MTL: return TF;
    switch(format) {
        MTL_FORMAT_MAPPING(M)
        default: return TextureFormat::kUnsupported;
    }
#undef M
}
MTLPixelFormat TextureFormatToMTLPixelFormat(TextureFormat format) {
    // Validate constants that can't be statically validated due to @available
    validate_mtl_pixelformats();

#define M(TF, MTL) case TF: return MTL;
    switch(format) {
        MTL_FORMAT_MAPPING(M)
        default: return MTLPixelFormatInvalid;
    }
#undef M
}

SkEnumBitMask<FormatSupport> MTLPixelFormatSupport(MTLGPUFamily family, MTLPixelFormat format) {
    const FamilyFeatureMap* familyMap;
    switch (family) {
        case MTLGPUFamilyApple2: familyMap = &kApple2; break;
        case MTLGPUFamilyApple3: familyMap = &kApple3; break;
        case MTLGPUFamilyApple4: familyMap = &kApple4; break;
        case MTLGPUFamilyApple5: familyMap = &kApple5; break;
        case MTLGPUFamilyApple6: familyMap = &kApple6; break;
        case MTLGPUFamilyApple7: familyMap = &kApple7; break;
        case MTLGPUFamilyApple8: familyMap = &kApple8; break;
        case MTLGPUFamilyApple9: familyMap = &kApple9; break;
        case MTLGPUFamilyMac2:   familyMap = &kMac2;   break;
        default:                 familyMap = nullptr;  break;
    }

    SkEnumBitMask<FormatSupport> supports;
    if (familyMap) {
        // This automatically returns NotAvailable for valid MTLPixelFormats that do not have an
        // associated TextureFormat.
        const TextureFormat tf = MTLPixelFormatToTextureFormat(format);
        const uint8_t features = familyMap->fFormatFeatures[(int) tf];

        if (features != NotAvailable) {
            // Every available pixel format can be used as a copy src and dst in Metal
            supports |= FormatSupport::kCopySrc | FormatSupport::kCopyDst;

            uint8_t msaaFlags = MSAA;
            if (TextureFormatIsDepthOrStencil(tf)) {
                // All available DS formats can be rendered into; Graphite never resolves DS
                // attachments so they support MSAA with just that feature flag
                supports |= FormatSupport::kRender;
            } else if ((features & Color) && (features & Blend)) {
                // Color formats require both being a color render target feature and having
                // blending to be useful.
                supports |= FormatSupport::kRender;
                // Graphite will also assume that it can use the texture format as the resolve
                // target for color attachments.
                msaaFlags |= Resolve;
            }

            if ((features & msaaFlags) == msaaFlags) {
                SkASSERT(supports & FormatSupport::kRender);
                supports |= FormatSupport::kMSAA;
            }

            // Graphite assumes you can filter in a shader (not just read texels)
            if (features & Filter) {
                supports |= FormatSupport::kSample;
            }

            // Graphite requires textures to be writable to be used for compute storage
            // (they are always readable).
            if (features & Write) {
                supports |= FormatSupport::kStorage;
            }
        }
    }

    return supports;
}

}  // namespace skgpu::graphite
