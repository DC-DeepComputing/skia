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
};

namespace skgpu::graphite {

struct MipLevel;

class Texture : public Resource {
public:
    ~Texture() override;

    int numSamples() const { return fInfo.numSamples(); }
    Mipmapped mipmapped() const { return fInfo.mipmapped(); }

    SkISize dimensions() const { return fDimensions; }
    const TextureInfo& textureInfo() const { return fInfo; }

    void setReleaseCallback(sk_sp<RefCntedCallback>);

    const char* getResourceType() const override { return "Texture"; }

    const Texture* asTexture() const override { return this; }

    // Called from Recorder::updateCompressedBackendTexture, attempts to write to the texture on the
    // host directly. If that is possible and done, `true` is returned.
    virtual bool updateDataOnHost(const SkColorInfo& colorInfo, SkSpan<const MipLevel> levels);
    virtual bool updateCompressedDataOnHost(const void* data, size_t dataSize);

protected:
    Texture(const SharedContext*,
            SkISize dimensions,
            const TextureInfo& info,
            bool isTransient,
            sk_sp<MutableTextureState> mutableState,
            Ownership);

    MutableTextureState* mutableState() const;

    void invokeReleaseProc() override;

    void onDumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump,
                                const char* dumpName) const override;

private:
    SkISize fDimensions;
    TextureInfo fInfo;
    sk_sp<MutableTextureState> fMutableState;
    sk_sp<RefCntedCallback> fReleaseCallback;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Texture_DEFINED
