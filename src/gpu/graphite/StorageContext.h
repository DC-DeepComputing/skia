/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_StorageContext_DEFINED
#define skgpu_graphite_StorageContext_DEFINED

#include "include/private/SkTDArray.h"
#include "src/core/SkTHash.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include <cstdint>
#include <limits>
#include <utility>

class SkGradientBaseShader;

namespace skgpu::graphite {

class DrawBufferManager;
class DrawPass;
class Recorder;
class TextureProxy;
class TextureProxyView;

class StorageContext {
public:
    StorageContext();
    ~StorageContext();

    // Resets cached gradient and vertex data. Should only occur at "organic" flush time
    // (e.g. canvas/recorder flush).
    void resetCache();

    // Stub, will call more than resetCache() in the future
    void reset() {
        this->resetCache();
    }

    std::pair<float*, int> allocateGradientData(int numStops, const SkGradientBaseShader* shader);

    // Used to determine the LCM across the stride/alignment requirements of all draws in a
    // drawPass, but does not write any data to the storageContext. Instead, at snapDrawPass() time,
    // the final LCM is used to align both the cached data in finalizePrecachedStorageData() and
    // the incoming appendVertices() stream. Should only be called prior to snapDrawPass().
    void recordAlignment(size_t stride, size_t align);

    // Appends `count * stride` bytes of vertex data into the CPU-side copy, aligned according to
    // `stride` and `align`. Returns the byte offset within the eventual storage buffer.
    uint32_t appendVertices(const void* data, size_t count, size_t stride, size_t align = 1);

    // Called at the beginning of snapping a draw pass. It pads the precached data size to the
    // running LCM so that subsequent appended data begins at a valid aligned offset. Does not
    // perform GPU uploads.
    void finalizePrecachedStorageData();

    static constexpr size_t kStructAlignment = 16;

    // Called after snapping the draw pass. It creates either a single mapped buffer or a fallback
    // texture containing the concatenated cached and append data.
    BindBufferInfo finalize(DrawBufferManager* bufferMgr,
                            Recorder* recorder = nullptr,
                            DrawPass* drawPass = nullptr);

    bool isEmpty() const { return fGradientCache.isEmpty() && fVertexData.empty(); }

    SkDEBUGCODE(int size() const { return fGradientCache.fGradientData.size(); })
    SkDEBUGCODE(int vertexSize() const { return fVertexData.size(); })

private:
    BindBufferInfo finalizeSSBO(DrawBufferManager* bufferMgr);
    TextureProxyView finalizeTexture(Recorder* recorder, DrawPass* drawPass);
    sk_sp<TextureProxy> createTextureProxy(Recorder* recorder, int width, int height);

private:
    struct GradientCache {
        static constexpr int kMaxGradientStops = 1024 * 1024;
        static constexpr int kMaxStorageFloats =
                static_cast<int>(std::numeric_limits<uint32_t>::max() / sizeof(float));

        skia_private::THashMap<const SkGradientBaseShader*, int> fLocalGradientOffsetCache;
        SkTDArray<float> fGradientData;
        size_t fGradientDataSize = 0;

        void reset();

        bool isEmpty() const { return fGradientData.empty(); }
    };

    SkDEBUGCODE(bool fGradientsFinalized = false;)

    GradientCache fGradientCache;

    SkTDArray<char> fVertexData;
    uint32_t fRunningLCM = 1;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_StorageContext_DEFINED
