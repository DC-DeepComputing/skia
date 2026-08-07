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
#include "src/gpu/graphite/ResourceTypes.h"

#include <cstdint>
#include <limits>
#include <utility>

class SkGradientBaseShader;

namespace skgpu::graphite {

class DrawBufferManager;

class StorageContext {
public:
    // Placeholder alignment, to be replaced by LCM across rendersteps in future.
    static constexpr size_t kStructAlignment = 16;

    StorageContext();
    ~StorageContext();

    // Resets cached gradient data. Should only occur at "organic" flush time (e.g. canvas/recorder
    // flush).
    void resetCache();

    // Stub, will call more than resetCache() in the future
    void reset() {
        this->resetCache();
    }

    std::pair<float*, int> allocateGradientData(int numStops, const SkGradientBaseShader* shader);

    int size() const;

    void finalizePrecachedStorageData();

    BindBufferInfo finalize(DrawBufferManager* bufferMgr);

private:
    struct GradientCache {
        static constexpr int kMaxGradientStops = 1024 * 1024;
        static constexpr int kMaxStorageFloats =
                static_cast<int>(std::numeric_limits<uint32_t>::max() / sizeof(float));

        skia_private::THashMap<const SkGradientBaseShader*, int> fLocalGradientOffsetCache;
        SkTDArray<float> fGradientData;
        size_t fGradientDataSize = 0;

        void reset();
    };

    SkDEBUGCODE(bool fGradientsFinalized = false;)

    GradientCache fGradientCache;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_StorageContext_DEFINED
