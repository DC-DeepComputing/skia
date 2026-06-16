/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Texture_DEFINED
#define skgpu_graphite_Texture_DEFINED

#include "include/core/SkSize.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceTypes.h"

class SkColorInfo;

namespace skgpu {
class MutableTextureState;
class RefCntedCallback;
enum class Budgeted : bool;
}

namespace skgpu::graphite {

class UploadSource;
enum class TextureUsageFlags : uint32_t;

class Texture : public Resource {
public:
    ~Texture() override;

    SampleCount sampleCount() const { return fInfo.sampleCount(); }
    Mipmapped mipmapped() const { return fInfo.mipmapped(); }

    SkISize dimensions() const { return fDimensions; }
    const TextureInfo& textureInfo() const { return fInfo; }

    void setReleaseCallback(sk_sp<RefCntedCallback>);

    const char* getResourceType() const override { return "Texture"; }

    const Texture* asTexture() const override { return this; }

    virtual bool canUploadOnHost() const { return false; }

    // With the assumption that the source data can upload on the host, attempts to write to the
    // texture on the host directly. Returns `false` only if driver calls fail.
    virtual bool uploadDataOnHost(const UploadSource& source);

    void setLabel(std::string_view label,
                  SkEnumBitMask<TextureUsageFlags> = {TextureUsageFlags::kNone}) override;

    // Scratch textures may store a bitmask of usages rather than a string label. Note that its
    // usages only align with the texture's label after Recording insertion, at which point these
    // usages can be used to generate a string.
    SkEnumBitMask<TextureUsageFlags> textureUsages() const { return fTextureUsages; }

    // Updating texture usages should only be called when it is threadsafe to update them (so either
    // by the Context during Recording insertion or when returning a non-shareable resource from the
    // cache), so we should not need to worry about thread safety in the implementation.
    void updateTextureUsages(SkEnumBitMask<TextureUsageFlags> usages) { fTextureUsages = usages; }

protected:
    Texture(const SharedContext*,
            SkISize dimensions,
            const TextureInfo&,
            bool isTransient,
            sk_sp<MutableTextureState>,
            Ownership,
            std::string_view label,
            SkEnumBitMask<TextureUsageFlags> = {TextureUsageFlags::kNone});

    MutableTextureState* mutableState() const;

    void invokeReleaseProc() override;

    void onDumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump,
                                const char* dumpName) const override;

private:
    static void AppendUsagesToLabel(std::string baseLabel, SkEnumBitMask<TextureUsageFlags>);

    SkISize fDimensions;
    TextureInfo fInfo;
    sk_sp<MutableTextureState> fMutableState;
    sk_sp<RefCntedCallback> fReleaseCallback;

    // Flags representing Texture usages. This allows for easier tracking of whether Texture
    // resources require label updates.
    SkEnumBitMask<TextureUsageFlags> fTextureUsages;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Texture_DEFINED
