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

    Protected isProtected() const override { return fInfo.isProtected(); }

    virtual bool canUploadOnHost() const { return false; }

    // With the assumption that the source data can upload on the host, attempts to write to the
    // texture on the host directly. Returns `false` only if driver calls fail.
    virtual bool uploadDataOnHost(const UploadSource& source);

    std::optional<std::string> onSetLabel(
            std::string_view label,
            SkEnumBitMask<TextureUsageFlags> usages = TextureUsageFlags::kNone) override {
        if (label == this->getLabel() && usages == fTextureUsages) {
            return std::nullopt; // No label change required.
        }

        fTextureUsages = usages;
        std::string updatedLabel(label);
        AppendUsagesToLabel(updatedLabel, fTextureUsages);
        return updatedLabel;
    }

    SkEnumBitMask<TextureUsageFlags> textureUsages() const { return fTextureUsages; }

    // Texture usage flags are assigned upon creation and should only be updated when it is thread-
    // safe to do so (i.e. by the Context during Recording insertion, or when returning a
    // non-shareable resource from the cache).
    void updateTextureUsages(SkEnumBitMask<TextureUsageFlags> usages) { fTextureUsages = usages; }

    static void AppendUsagesToLabel(std::string label, SkEnumBitMask<TextureUsageFlags>);

protected:
    Texture(const SharedContext*,
            SkISize dimensions,
            const TextureInfo&,
            bool isTransient,
            sk_sp<MutableTextureState>,
            Ownership,
            SkEnumBitMask<TextureUsageFlags>,
            std::string_view label);

    MutableTextureState* mutableState() const;

    void invokeReleaseProc() override;

    void onDumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump,
                                const char* dumpName) const override;

private:
    SkISize fDimensions;
    TextureInfo fInfo;
    sk_sp<MutableTextureState> fMutableState;
    sk_sp<RefCntedCallback> fReleaseCallback;

    // Flags representing known Texture usages. This allows us to more efficiently determine whether
    // the Texture needs a label update or not before performing string generation and comparison.
    SkEnumBitMask<TextureUsageFlags> fTextureUsages;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Texture_DEFINED
