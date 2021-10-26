/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_UniformManager_DEFINED
#define skgpu_UniformManager_DEFINED

#include "experimental/graphite/include/private/GraphiteTypesPriv.h"
#include "experimental/graphite/src/Uniform.h"
#include "include/core/SkSpan.h"

namespace skgpu {

enum class Layout {
    kStd140,
    kStd430,
    kMetal, /** This is our own self-imposed layout we use for Metal. */
};

class UniformManager {
public:
    UniformManager(Layout layout);

    uint32_t writeUniforms(SkSpan<const Uniform> definitions,
                           void** srcs,
                           uint32_t* offsets,
                           void *dst);

private:
    Layout fLayout;
};

} // namespace skgpu

#endif // skgpu_UniformManager_DEFINED
