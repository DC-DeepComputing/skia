/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ResourceTypes_DEFINED
#define skgpu_graphite_ResourceTypes_DEFINED

#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkMathPriv.h"

#include <vector>

#include <vector>

#include <vector>

namespace skgpu::graphite {

class Buffer;

enum class DepthStencilFlags : int {
    kNone = 0b000,
    kDepth = 0b001,
    kStencil = 0b010,
    kDepthStencil = kDepth | kStencil,
};
SK_MAKE_BITMASK_OPS(DepthStencilFlags)

/**
 * What a GPU buffer will be used for
 */
enum class BufferType : int {
    kVertex,
    kIndex,
    kXferCpuToGpu,
    kXferGpuToCpu,
    kUniform,
    kStorage,

    // GPU-only buffer types
    kIndirect,
    kVertexStorage,
    kIndexStorage,

    kLast = kIndexStorage,
};
static const int kBufferTypeCount = static_cast<int>(BufferType::kLast) + 1;

/**
 * Data layout requirements on host-shareable buffer contents.
 */
enum class Layout {
    kInvalid = 0,
    kStd140,
    kStd430,
    kMetal,
};

static constexpr const char* LayoutString(Layout layout) {
    switch(layout) {
        case Layout::kStd140:  return "std140";
        case Layout::kStd430:  return "std430";
        case Layout::kMetal:   return "metal";
        case Layout::kInvalid: return "invalid";
    }
    SkUNREACHABLE;
}

/**
 * Indicates the intended access pattern over resource memory. This is used to select the most
 * efficient memory type during resource creation based on the capabilities of the platform.
 *
 * This is only a hint and the actual memory type will be determined based on the resource type and
 * backend capabilities.
 */
enum class AccessPattern : int {
    // GPU-only memory does not need to support reads/writes from the CPU. GPU-private memory will
    // be preferred if the backend supports an efficient private memory type.
    kGpuOnly,

    // The resource needs to be CPU visible, e.g. for read-back or as a copy/upload source.
    kHostVisible,
};

/**
 * Determines whether the contents of a GPU buffer sub-allocation gets cleared to 0 before being
 * used in a GPU command submission.
 */
enum class ClearBuffer : bool {
    kNo = false,
    kYes = true,
};

/**
 * Must the contents of the Resource be preserved af a render pass or can a more efficient
 * representation be chosen when supported by hardware.
 */
enum class Discardable : bool {
    kNo = false,
    kYes = true
};

enum class Ownership {
    kOwned,
    kWrapped,
};

/** Uniquely identifies the type of resource that is cached with a GraphiteResourceKey. */
using ResourceType = uint32_t;

/**
 * Can the resource be held by multiple users at the same time?
 * For example, stencil buffers, pipelines, etc.
 */
enum class Shareable : bool {
    kNo = false,
    kYes = true,
};

/**
 * This enum is used to notify the ResourceCache which type of ref just dropped to zero on a
 * Resource.
 */
enum class LastRemovedRef {
    kUsage,
    kCommandBuffer,
    kCache,
};

/*
 * Struct that can be passed into bind buffer calls on the CommandBuffer. The ownership of the
 * buffer and its usage in command submission must be tracked by the caller (e.g. as with
 * buffers created by DrawBufferManager).
 */
struct BindBufferInfo {
    const Buffer* fBuffer = nullptr;
    size_t fOffset = 0;

    operator bool() const { return SkToBool(fBuffer); }

    bool operator==(const BindBufferInfo& o) const {
        return fBuffer == o.fBuffer && (!fBuffer || fOffset == o.fOffset);
    }
    bool operator!=(const BindBufferInfo& o) const { return !(*this == o); }
};

/*
 * Struct that can be passed into bind uniform buffer calls on the CommandBuffer.
 * It is similar to BindBufferInfo with additional fBindingSize member.
 */
struct BindUniformBufferInfo : public BindBufferInfo {
    // TODO(b/308933713): Add size to BindBufferInfo instead
    uint32_t fBindingSize = 0;

    bool operator==(const BindUniformBufferInfo& o) const {
        return BindBufferInfo::operator==(o) && (!fBuffer || fBindingSize == o.fBindingSize);
    }
    bool operator!=(const BindUniformBufferInfo& o) const { return !(*this == o); }
};

/**
 * Represents a buffer region that should be cleared to 0. A ClearBuffersTask does not take an
 * owning reference to the buffer it clears. A higher layer is responsible for managing the lifetime
 * and usage refs of the buffer.
 */
struct ClearBufferInfo {
    const Buffer* fBuffer = nullptr;
    size_t fOffset = 0;
    size_t fSize = 0;

    operator bool() const { return SkToBool(fBuffer); }
};

enum class SamplerMutability {
    kDynamicSampler,                 // "regular" dynamic sampler
    kImmutableSamplerKnownFormat,    // immutable sampler using a known format
    kImmutableSamplerExternalFormat, // immutable sampler using an external format

    kLast = kImmutableSamplerExternalFormat,
};
static constexpr int kSamplerMutabilityCount = static_cast<int>(SamplerMutability::kLast) + 1;

struct SamplerConversionInfo {
    SamplerMutability fMutability = SamplerMutability::kDynamicSampler;
    // If the sampler requires YCbCr conversion, backends can place that information within
    // this 16-bit value.
    uint8_t fNonFormatYcbcrConversionInfo = 0;
    // fFormat represents known OR external format numerical representation depending upon whether
    // SkSamplerMutability dictates the usage of a known or external format.
    uint64_t fFormat = 0;
    uint32_t fFormatFeatures = 0;
};


/**
 * Struct used to describe how a Texture/TextureProxy/TextureProxyView is sampled.
 */
struct SamplerDesc {
    static_assert(kSkTileModeCount   <= 4 && kSkFilterModeCount <= 2 && kSkMipmapModeCount <= 3);
    SamplerDesc(const SkSamplingOptions& samplingOptions,
                const SkTileMode tileModes[2],
                const SamplerConversionInfo conversionInfo = {})
            : fDesc((static_cast<int>(tileModes[0])               << kTileModeXShift     ) |
                    (static_cast<int>(tileModes[1])               << kTileModeYShift     ) |
                    (static_cast<int>(samplingOptions.mipmap)     << kMipmapModeShift    ) |
                    (static_cast<int>(conversionInfo.fMutability) << kMutabilityShift    ) |
                    (conversionInfo.fNonFormatYcbcrConversionInfo << kConversionInfoShift) )
            , fFormat(conversionInfo.fFormat)
            , fFormatFeatures(conversionInfo.fFormatFeatures) {
        // Cubic sampling is handled in a shader, with the actual texture sampled by with NN,
        // but that is what a cubic SkSamplingOptions is set to if you ignore 'cubic', which let's
        // us simplify how we construct SamplerDec's from the options passed to high-level draws.
        SkASSERT(!samplingOptions.useCubic || (samplingOptions.filter == SkFilterMode::kNearest &&
                                               samplingOptions.mipmap == SkMipmapMode::kNone));

        // Basic, backend-agnostic sampler information can fit within one uint32_t.
        // TODO: Add aniso value when used.
        static_assert(sizeof(uint32_t) == 4 && sizeof(uint8_t) == 1);
        static_assert(sizeof(conversionInfo.fNonFormatYcbcrConversionInfo) == sizeof(uint8_t));
        static_assert((kMutabilityShift + kNumMutabilityBits + sizeof(uint8_t)) <= 32);
    }

    SamplerDesc(const SamplerDesc&) = default;

    bool operator==(const SamplerDesc& o) const {
        return o.fDesc == fDesc && o.fFormat == fFormat && fFormatFeatures == o.fFormatFeatures;
    }

    bool operator!=(const SamplerDesc& o) const { return !(*this == o); }

    operator SkString() const { // TODO: Explore better representations of the sampler desc
        SkString uniqueRep {fDesc};
        uniqueRep.appendU64(fFormat);
        uniqueRep.appendU32(fFormatFeatures);
        return uniqueRep;
    }

    SkTileMode tileModeX() const { return static_cast<SkTileMode>((fDesc >> 0) & 0b11); }
    SkTileMode tileModeY() const { return static_cast<SkTileMode>((fDesc >> 2) & 0b11); }
    uint32_t desc() const { return fDesc; }

    // NOTE: returns the HW sampling options to use, so a bicubic SkSamplingOptions will become
    // nearest-neighbor sampling in HW.
    SkSamplingOptions samplingOptions() const {
        // TODO: Add support for anisotropic filtering
        SkFilterMode filter = static_cast<SkFilterMode>((fDesc >> 4) & 0b01);
        SkMipmapMode mipmap = static_cast<SkMipmapMode>((fDesc >> 5) & 0b11);
        return SkSamplingOptions(filter, mipmap);
    }

    uint64_t format() const { return fFormat; }
    uint32_t formatFeatures() const { return fFormatFeatures; }

    // These are public such that backends can bitshift data in order to determine whatever
    // sampler qualities they need from fDesc.
    static constexpr int kNumTileModeBits   = SkNextLog2_portable(int(SkTileMode::kLastTileMode)+1);
    static constexpr int kNumFilterModeBits = SkNextLog2_portable(int(SkFilterMode::kLast)+1);
    static constexpr int kNumMipmapModeBits = SkNextLog2_portable(int(SkMipmapMode::kLast)+1);
    static constexpr int kNumMutabilityBits = SkNextLog2_portable(int(SamplerMutability::kLast)+1);

    static constexpr int kTileModeXShift      = 0;
    static constexpr int kTileModeYShift      = kTileModeXShift  + kNumTileModeBits;
    static constexpr int kFilterModeShift     = kTileModeYShift  + kNumTileModeBits;
    static constexpr int kMipmapModeShift     = kFilterModeShift + kNumFilterModeBits;
    static constexpr int kMutabilityShift     = kMipmapModeShift + kNumMutabilityBits;
    static constexpr int kConversionInfoShift = kMutabilityShift + sizeof(uint16_t);

private:
    uint32_t fDesc;
    // Data field populated by backend Caps which stored texture format information (needed for
    // YCbCr sampling). Only relevant when using immutable samplers (ie, sampler mutability !=
    // SamplerMutability::kDynamicSampler). Otherwise, can be ignored.
    // TODO: Optimize such that we store an int32 as opposed to 64 when using a known format.
    uint64_t fFormat = 0;
    uint32_t fFormatFeatures = 0;
};

};  // namespace skgpu::graphite

#endif // skgpu_graphite_ResourceTypes_DEFINED
