/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_CommandBuffer_DEFINED
#define skgpu_CommandBuffer_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu {

class CommandBuffer : public SkRefCnt {
public:
    ~CommandBuffer() override {}

protected:
    CommandBuffer();

private:
};

} // namespace skgpu

#endif // skgpu_Gpu_DEFINED
