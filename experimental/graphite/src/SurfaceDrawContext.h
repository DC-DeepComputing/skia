/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_SurfaceDrawContext_DEFINED
#define skgpu_SurfaceDrawContext_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu {

class SurfaceDrawContext : public SkRefCnt {
public:
    SurfaceDrawContext();
    ~SurfaceDrawContext() override;

private:
};

} // namespace skgpu

#endif // skgpu_SurfaceDrawContext_DEFINED

