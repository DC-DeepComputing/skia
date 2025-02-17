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
class SkStream;
class SkWStream;

namespace skgpu::graphite {

class TextureInfoData;

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
    //
    // Each backend subclass must expose to TextureInfo:
    //   static constexpr BackendApi kBackend;
    //   Protected isProtected() const;
    //   bool isMemoryless() const;
    class Data {
    public:
        virtual ~Data() = default;

        Data(uint32_t sampleCount, skgpu::Mipmapped mipmapped)
                : fSampleCount(sampleCount)
                , fMipmapped(mipmapped) {}

        Data() = default;
        Data(const Data&) = default;

        Data& operator=(const Data&) = default;

        // NOTE: These fields are accessible via the backend-specific subclasses.
        uint32_t fSampleCount = 1;
        Mipmapped fMipmapped = Mipmapped::kNo;

    private:
        friend class TextureInfo;
        friend class TextureInfoPriv;

        // TODO(): These will be consolidated into a single function returning skgpu::TextureFormat
        virtual size_t bytesPerPixel() const = 0;
        virtual SkTextureCompressionType compressionType() const = 0;
        virtual uint32_t viewFormat() const = 0;

        virtual SkString toBackendString() const = 0;

        virtual void copyTo(AnyTextureInfoData&) const = 0;
        // Passed in TextureInfo will have data of the same backend type and subclass, and
        // base properties of Data have already been checked for equality/compatibility.
        virtual bool equals(const TextureInfo& that) const = 0;
        virtual bool isCompatible(const TextureInfo& that) const = 0;

        virtual bool serialize(SkWStream*) const { return false; }
        virtual bool deserialize(SkStream*) { return false; }
    };

public:
    TextureInfo() = default;
    ~TextureInfo() = default;

    TextureInfo(const TextureInfo&);
    TextureInfo& operator=(const TextureInfo&);

    bool operator==(const TextureInfo&) const;
    bool operator!=(const TextureInfo& that) const { return !(*this == that); }

    bool isValid() const { return fData.has_value(); }
    BackendApi backend() const {
        SkASSERT(fData.has_value() || fBackend == BackendApi::kUnsupported);
        return fBackend;
    }

    uint32_t numSamples() const { return fData->fSampleCount; }
    Mipmapped mipmapped() const { return fData->fMipmapped; }
    Protected isProtected() const { return fProtected; }
    bool isMemoryless() const { return fMemoryless; }

    // Return true if `that` describes a texture that is compatible with this info (e.g. the other
    // texture can fulfill a promise for a texture with this info).
    bool isCompatible(const TextureInfo& that) const;

    // Return a string containing the full description of this TextureInfo.
    SkString toString() const;

private:
    friend class TextureInfoPriv;

    template <typename BackendTextureData>
    TextureInfo(const BackendTextureData& data)
            : fBackend(BackendTextureData::kBackend)
            , fProtected(data.isProtected())
            , fMemoryless(data.isMemoryless()) {
        fData.emplace<BackendTextureData>(data);
    }

    // TODO(): Once TextureInfo stores a unified skgpu::TextureFormat, these can be removed.
    friend size_t ComputeSize(SkISize dimensions, const TextureInfo&);  // for bytesPerPixel
    size_t bytesPerPixel() const { return fData.has_value() ? fData->bytesPerPixel() : 0; }
    SkTextureCompressionType compressionType() const {
        return fData.has_value() ? fData->compressionType() : SkTextureCompressionType::kNone;
    }

    skgpu::BackendApi  fBackend = BackendApi::kUnsupported;
    AnyTextureInfoData fData;

    // Derived properties from the backend data, cached to avoid a virtual function call
    Protected fProtected = Protected::kNo;
    bool fMemoryless     = false;
};

}  // namespace skgpu::graphite

#endif  //skgpu_graphite_TextureInfo_DEFINED
