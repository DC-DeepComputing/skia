/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnBuffer_DEFINED
#define skgpu_graphite_DawnBuffer_DEFINED

#include "webgpu/webgpu_cpp.h"

#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/dawn/DawnTypes.h"
#include "src/gpu/graphite/Buffer.h"

namespace skgpu::graphite {

class DawnUniformBufferBindGroupCacheKeepAlive;

class DawnBuffer : public Buffer {
public:
    const wgpu::Buffer& dawnBuffer() const { return fBuffer; }

private:

    void onMap() override;
    void onUnmap() override;

    void freeGpuData() override;

    wgpu::Buffer fBuffer;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DawnBuffer_DEFINED
