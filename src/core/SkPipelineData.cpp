/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"
#include "src/core/SkPipelineData.h"

void SkPipelineData::add(sk_sp<SkUniformData> uniforms) {
    SkASSERT(uniforms && uniforms->count());
    fUniformDataBlock.add(std::move(uniforms));
}

size_t SkPipelineData::UniformDataBlock::totalUniformSize() const {
    size_t total = 0;

    // TODO: It seems like we need to worry about alignment between the separate sets of uniforms
    for (auto& u : fUniformData) {
        total += u->dataSize();
    }

    return total;
}

int SkPipelineData::UniformDataBlock::numUniforms() const {
    int total = 0;

    for (auto& u : fUniformData) {
        total += u->count();
    }

    return total;
}

bool SkPipelineData::UniformDataBlock::operator==(const SkPipelineData::UniformDataBlock& other) const {
    if (fUniformData.size() != other.fUniformData.size()) {
        return false;
    }

    for (size_t i = 0; i < fUniformData.size(); ++i) {
        if (*fUniformData[i] != *other.fUniformData[i]) {
            return false;
        }
    }

    return true;
}

size_t SkPipelineData::UniformDataBlock::hash1() const {
    int32_t hash = 0;

    for (auto& u : fUniformData) {
        hash = SkOpts::hash_fn(u->data(), u->dataSize(), hash);
    }

    return hash;
}

#ifdef SK_GRAPHITE_ENABLED
void SkPipelineData::add(const SkSamplingOptions& sampling,
                         const SkTileMode tileModes[2],
                         sk_sp<skgpu::TextureProxy> proxy) {
    fTextureDataBlock.add(sampling, tileModes, std::move(proxy));
}

bool SkPipelineData::TextureDataBlock::TextureInfo::operator==(const TextureInfo& other) const {
    return fProxy == other.fProxy &&
           fSamplingOptions == other.fSamplingOptions &&
           fTileModes[0] == other.fTileModes[0] &&
           fTileModes[1] == other.fTileModes[1];
}

bool SkPipelineData::TextureDataBlock::operator==(const SkPipelineData::TextureDataBlock& other) const {
    if (fTextureData.size() != other.fTextureData.size()) {
        return false;
    }

    for (size_t i = 0; i < fTextureData.size(); ++i) {
        if (fTextureData[i] != other.fTextureData[i]) {
            return false;
        }
    }

    return true;
}

size_t SkPipelineData::TextureDataBlock::hash1() const {
    int32_t hash = 0;

    for (auto& d : fTextureData) {
        uint32_t samplerKey = d.samplingAsKey();
        hash = SkOpts::hash_fn(&samplerKey, sizeof(samplerKey), hash);

        // Because the lifetime of the TextureDataCache is for just one Recording and the
        // TextureDataBlocks hold refs on their proxies, we can just use the proxy's pointer
        // for the hash here. This is a bit sloppy though in that it would be nice if proxies backed
        // by the same scratch texture hashed the same (it is tough to see how we could do that
        // at DrawPass creation time though).
        hash = SkOpts::hash_fn(d.fProxy.get(), sizeof(skgpu::TextureProxy*), hash);
    }

    return hash;
}

#endif
