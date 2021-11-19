/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ShaderCaps_DEFINED
#define skgpu_ShaderCaps_DEFINED

#include "src/sksl/SkSLUtil.h"

namespace skgpu {
namespace mtl {
class Caps;
}

class ShaderCaps : public SkSL::ShaderCaps {
public:
    ShaderCaps() {}

private:

    friend class skgpu::mtl::Caps;
};

} // namespace skgpu

#endif // skgpu_ShaderCaps_DEFINED
