/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_TextureInfo_DEFINED
#define skgpu_graphite_TextureInfo_DEFINED

#include "include/core/SkString.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/private/base/SkAPI.h"
#include "include/private/base/SkAnySubclass.h"

struct SkISize;

namespace skgpu::graphite {

/**
 * TextureInfo is a backend-agnostic wrapper around the properties of a texture, sans dimensions.
 * It is designed this way to be compilable w/o bringing in a specific backend's build files, and
 * without requiring heap allocations of virtual types.
 */
class SK_API TextureInfo {
    class Data;
    friend class MtlTextureInfo;
    friend class DawnTextureInfo;
    friend class VulkanTextureInfo;

    // Size is the smallest of the Data subclass assuming a 64-bit compiler.
    inline constexpr static size_t kMaxSubclassSize = 112;
    using AnyTextureInfoData = SkAnySubclass<Data, kMaxSubclassSize>;

    // Base properties for all backend-specific properties. Clients managing textures directly
    // should use the public subclasses of Data directly, e.g. MtlTextureInfo/DawnTextureInfo.
    class Data {
    public:
        Data(uint32_t sampleCount, skgpu::Mipmapped mipmapped)
                : fSampleCount(sampleCount)
                , fMipmapped(mipmapped) {}

        Data() = default;
        Data(const Data&) = default;

        virtual ~Data() = default;

        uint32_t fSampleCount = 1;
        Mipmapped fMipmapped = Mipmapped::kNo;

    protected:
        using AnyTextureInfoData = TextureInfo::AnyTextureInfoData;

    private:
        friend class TextureInfo;

        virtual skgpu::BackendApi type() const = 0;

        virtual size_t bytesPerPixel() const = 0;
        virtual SkTextureCompressionType compressionType() const = 0;
        virtual Protected isProtected() const = 0;
        virtual bool isMemoryless() const = 0;
        virtual uint32_t viewFormat() const = 0;

        virtual SkString toBackendString() const = 0;

        virtual void copyTo(AnyTextureInfoData&) const = 0;
        // Passed in TextureInfo will have data of the same backend type and subclass
        virtual bool equals(const TextureInfo& that) const = 0;
        virtual bool isCompatible(const TextureInfo& that) const = 0;
    };

public:
    TextureInfo();
    ~TextureInfo();
    TextureInfo(const TextureInfo&);
    TextureInfo& operator=(const TextureInfo&);

    bool operator==(const TextureInfo&) const;
    bool operator!=(const TextureInfo& that) const { return !(*this == that); }

    bool isValid() const { return fData.has_value(); }
    BackendApi backend() const {
        return this->isValid() ? fData->type() : BackendApi::kUnsupported;
    }

    uint32_t numSamples() const { return fData->fSampleCount; }
    Mipmapped mipmapped() const { return fData->fMipmapped; }
    Protected isProtected() const { return fData->isProtected(); }

    SkTextureCompressionType compressionType() const { return fData->compressionType(); }
    bool isMemoryless() const { return fData->isMemoryless(); }
    size_t bytesPerPixel() const { return fData->bytesPerPixel(); }

    bool isCompatible(const TextureInfo& that) const;

    // Return a string containing the full description of this TextureInfo.
    SkString toString() const;
    // Return a string containing only the info relevant for its use as a RenderPass attachment.
    SkString toRPAttachmentString() const;


private:
    friend class TextureInfoPriv;

    template <typename SomeTextureInfoData>
    TextureInfo(const SomeTextureInfoData& textureInfoData) {
        fData.emplace<SomeTextureInfoData>(textureInfoData);
    }

    AnyTextureInfoData fData;
    // Protected fProtected;
};

}  // namespace skgpu::graphite

#endif  //skgpu_graphite_TextureInfo_DEFINED
