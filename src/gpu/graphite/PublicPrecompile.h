/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PublicPrecompile_DEFINED
#define skgpu_graphite_PublicPrecompile_DEFINED

#if 1
#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/GraphiteTypes.h"
// TODO: this header should be moved to include/gpu/graphite once the precompilation API
// is made public
namespace skgpu::graphite {

class Context;
class GraphicsPipelineDesc;
class PaintOptions;
struct RenderPassDesc;
class RuntimeEffectDictionary;
class ThreadSafeProxy;

/*
 * TODO: Rather than passing in a pipelineDesc and renderPassDesc we need to add an
 * opaque serializable object that contains the same information.
 */
bool Precompile1(sk_sp<ThreadSafeProxy>,
                RuntimeEffectDictionary* rteDict,
                const GraphicsPipelineDesc& pipelineDesc,
                const RenderPassDesc& renderPassDesc);

} // namespace skgpu::graphite

#endif

#endif // skgpu_graphite_PublicPrecompile_DEFINED
