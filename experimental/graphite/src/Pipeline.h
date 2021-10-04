/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Pipeline_DEFINED
#define skgpu_Pipeline_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu {

class RenderPipeline : public SkRefCnt {
public:
    ~RenderPipeline() override;

protected:
    RenderPipeline();

private:
};

// TODO: add ComputePipeline

} // namespace skgpu

#endif // skgpu_Pipeline_DEFINED
