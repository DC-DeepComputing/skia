/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnBuffer_DEFINED
#define skgpu_graphite_DawnBuffer_DEFINED

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/dawn/DawnAsyncWait.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

namespace skgpu::graphite {

class DawnBuffer : public Buffer {
public:
    static sk_sp<DawnBuffer> Make(const DawnSharedContext*,
                                  size_t,
                                  BufferType,
                                  AccessPattern,
                                  std::string_view label);

    bool isUnmappable() const override;

    const wgpu::Buffer& dawnBuffer() const { return fBuffer; }

private:
    DawnBuffer(const DawnSharedContext*,
               size_t,
               wgpu::Buffer,
               void* mapAtCreationPtr,
               std::string_view label);

#if defined(__EMSCRIPTEN__)
    bool prepareForReturnToCache(const std::function<void()>& takeRef) override;
    void onAsyncMap(GpuFinishedProc, GpuFinishedContext) override;
#endif
    void onMap() override;
    void onUnmap() override;

    template <typename StatusT, typename MessageT>
    void mapCallback(StatusT status, MessageT message);

    void freeGpuData() override;

    const DawnSharedContext* dawnSharedContext() const {
        return static_cast<const DawnSharedContext*>(this->sharedContext());
    }

    void setBackendLabel(char const* label) override;

    wgpu::Buffer fBuffer;
    SkMutex fAsyncMutex;
    skia_private::TArray<sk_sp<RefCntedCallback>> fAsyncMapCallbacks SK_GUARDED_BY(fAsyncMutex);
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnBuffer_DEFINED
