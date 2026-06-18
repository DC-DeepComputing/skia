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

    SkEnumBitMask<TextureUsageFlags> textureUsages() const { return fTextureUsages; }

    // This following methods should only be called when it is threadsafe to update them (so either
    // at creation time, by the Context during Recording insertion, or when returning a
    // non-shareable resource from the cache), so we should not need to worry about thread safety.
    void updateTextureUsages(SkEnumBitMask<TextureUsageFlags> usages) { fTextureUsages = usages; }
    // This method allows us to more efficiently determine if a label update is required by checking
    // if the Texture's label without any texture usage flag string components is equal to that of
    // `proxyLabelPortion`
    void updateLabelIfProxyInfoDiffers(std::string_view proxyLabelPortion,
                                       SkEnumBitMask<TextureUsageFlags>);

    static void AppendUsagesToLabel(std::string label, SkEnumBitMask<TextureUsageFlags>);

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
    // This should only be called iff we have already determined that the label (or the components
    // that would make up the Texture's label) differs and requires an update.
    void updateLabelInternal(std::string_view label, SkEnumBitMask<TextureUsageFlags>);

    SkISize fDimensions;
    TextureInfo fInfo;
    sk_sp<MutableTextureState> fMutableState;
    sk_sp<RefCntedCallback> fReleaseCallback;

    // Flags representing Texture usages. This allows for easier tracking of whether Texture
    // resources require label updates.
    SkEnumBitMask<TextureUsageFlags> fTextureUsages;
    // Note that one Scratch texture may be instantiated from different proxies. The expected label
    // structure is: "ScratchTexture_X_Y", where X = each proxy's dynamic string label separted with
    // an underscore, and Y = each proxy's usage flag separated with an underscore.
    // For example, consider a scratch texture that is instantiated by a proxy with custom label
    // "A" and by a proxy with TextureUsageFlags::kAsyncReadPixelsFallback. The expected label for
    // that scratch texture after inserting the Recording would be:
    // "ScratchTexture_A_AsyncReadPixelsFallback".
    //
    // Storing the component of the label that excludes the usage flags allows for easier comparison
    std::string_view  fProxyStringLabelPortion;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Texture_DEFINED
