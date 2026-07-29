/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_StorageContext_DEFINED
#define skgpu_graphite_StorageContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkTHash.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include <cstdint>
#include <limits>
#include <utility>

class SkGradientBaseShader;

namespace skgpu::graphite {

class DrawBufferManager;

class StorageContext {
public:
    static constexpr size_t kStructAlignment = 16;

    struct AppendedAllocation {
        VertexWriter fWriter;
        uint32_t     fOffset;
    };

    StorageContext();
    ~StorageContext();

    // Resets cached gradient data. Should only occur at "organic" flush time (e.g. canvas/recorder
    // flush).
    void resetCache();

    // Resets appended vertex/instance storage. Called when finalizing a DrawPass snap.
    void resetAppendedStorage();

    void reset() {
        this->resetCache();
        this->resetAppendedStorage();
    }

    std::pair<float*, int> allocateGradientData(int numStops, const SkGradientBaseShader* shader);

    int size() const;

    void finalizeGradientStorage();

    void finalizeCacheStorageData() {
        this->finalizeGradientStorage();
    }

    [[maybe_unused]] AppendedAllocation append(size_t count, size_t stride);

    [[maybe_unused]] AppendedAllocation appendVertexStorage(size_t count, size_t stride) {
        return this->append(count, stride);
    }

    BindBufferInfo finalize(DrawBufferManager* bufferMgr);

private:
    struct GradientCache {
        static constexpr int kMaxGradientStops = 1024 * 1024;
        static constexpr int kMaxStorageFloats =
                static_cast<int>(std::numeric_limits<uint32_t>::max() / sizeof(float));

        skia_private::THashMap<const SkGradientBaseShader*, int> fLocalGradientOffsetCache;
        skia_private::STArray<4, const SkGradientBaseShader*> fContextGradientsList;
        SkTDArray<float> fGradientData;
        size_t fGradientDataSize = 0;

        void reset();
    };

    SkDEBUGCODE(bool fGradientsFinalized = false;)

    GradientCache fGradientCache;
    SkTDArray<char> fAppendedData;
    std::optional<BindBufferInfo> fBufferInfo;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_StorageContext_DEFINED
