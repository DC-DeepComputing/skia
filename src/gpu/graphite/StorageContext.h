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
    static constexpr size_t kStructAlignment = 16;

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

    // Records the required alignment for a vertex stride and alignment without appending data.
    // This updates the running LCM used when finalizing precached storage data.
    void recordAlignment(size_t stride, size_t align = 1);

    // Appends `count * stride` bytes of vertex data into the CPU-side copy, aligned according to
    // `stride` and `align`. Returns the byte offset within the storage buffer.
    uint32_t appendVertices(const void* data, size_t count, size_t stride, size_t align = 1);

    void finalizePrecachedStorageData();

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
