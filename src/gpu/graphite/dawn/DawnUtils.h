/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnUtils_DEFINED
#define skgpu_graphite_DawnUtils_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/ports/SkCFObject.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/sksl/ir/SkSLProgram.h"

#include "webgpu/webgpu_cpp.h"

namespace skgpu {
class ShaderErrorHandler;
}

namespace SkSL {
    class Compiler;
}

namespace skgpu::graphite {
class DawnSharedContext;

bool DawnFormatIsDepthOrStencil(wgpu::TextureFormat);
bool DawnFormatIsDepth(wgpu::TextureFormat);
bool DawnFormatIsStencil(wgpu::TextureFormat);

wgpu::TextureFormat DawnDepthStencilFlagsToFormat(SkEnumBitMask<DepthStencilFlags>);

} // namespace skgpu::graphite

#endif // skgpu_graphite_DawnUtils_DEFINED
