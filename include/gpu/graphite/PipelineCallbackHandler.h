/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PipelineCallbackHandler_DEFINED
#define PipelineCallbackHandler_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "src/base/SkSpinlock.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkTHash.h"

#include "include/gpu/graphite/PrecompileContext.h"

#include <vector>

class SkData;

namespace skgpu::graphite {
    class PrecompileContext;
}

namespace skiatools::graphite {


// This is intended to be an example of a Precompilation Callback handler. For DM it collects
// all the Android-style keys that are used by a given source (e.g., gm, or skp) and uses
// them in resetAndRecreatePipelines to recreate the Pipelines.
class SK_API PipelineCallBackHandler {
public:
    PipelineCallBackHandler();
    ~PipelineCallBackHandler();

    static void CallBack(void* data, sk_sp<SkData> androidStyleKey, const std::string& label) {
        PipelineCallBackHandler* handler = reinterpret_cast<PipelineCallBackHandler*>(data);

        handler->add(std::move(androidStyleKey), label);
    }

    void setFoo(std::unique_ptr<skgpu::graphite::PrecompileContext> foo) {
        fPCC = std::move(foo);
    }

    // Add an Android-style key to the map
    void add(sk_sp<SkData> androidStyleKey, const std::string& label) SK_EXCLUDES(fSpinLock);

    // Retrieve all the unique collected keys
    void retrieve(std::vector<sk_sp<SkData>>*) SK_EXCLUDES(fSpinLock);

    void reset() SK_EXCLUDES(fSpinLock);

    int numKeys() const SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{ fSpinLock };
        return fMap.count();
    }

    void dumpAll() SK_EXCLUDES(fSpinLock);

private:
    mutable SkSpinlock fSpinLock;

    struct Type7 {
        Type7(int id, sk_sp<SkData> data) : fID(id), fData(std::move(data)) {}

        const int fID;
        sk_sp<SkData> fData;
        int fHits = 0;
    };

    struct SkDataKey {
        SkDataKey(Type7* t) : fData(t->fData.get()) {}
        SkDataKey(SkData* d) : fData(d) {}

        static SkDataKey GetKey(Type7& t) { return { t.fData.get() }; }
        static uint32_t Hash(const SkDataKey& k) { return k.hash(); }

        bool operator==(const SkDataKey& other) const { return fData->equals(other.fData); }
        uint32_t hash() const { return SkChecksum::Hash32(fData->data(), fData->size()); }

        const SkData* fData;
    };



    skia_private::THashTable<Type7, SkDataKey, SkDataKey> fMap SK_GUARDED_BY(fSpinLock);
    std::unique_ptr<skgpu::graphite::PrecompileContext> fPCC;
    FILE* fFile;
};

}  // namespace skiatools::graphite

#endif  // PipelineCallbackHandler_DEFINED
