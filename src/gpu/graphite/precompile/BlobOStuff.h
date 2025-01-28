/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_BlobOStuff_DEFINED
#define skgpu_graphite_precompile_BlobOStuff_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"
#include "src/gpu/ResourceKey.h"
//#include "include/core/SkSpan.h"

#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/RenderPassDesc.h"

class SkStream;
class SkWStream;

namespace skgpu::graphite {

class ShaderCodeDictionary;
class SharedContext;

// It seems like this could just be a name space that contains all the static helpers.
// As it stands it helps to orchestrate the creation of the Android-specific serializable
// Precompilation key.
class BlobOStuff {
public:

    BlobOStuff(const GraphicsPipelineDesc& pipelineDesc, const RenderPassDesc& renderPassDesc)
               : fPipelineDesc(pipelineDesc)
               , fRenderPassDesc(renderPassDesc) {}
    BlobOStuff() {}

    static std::unique_ptr<BlobOStuff> MakeFromStream(const Caps*,
                                                      ShaderCodeDictionary*,
                                                      SkStream* stream);

    static std::unique_ptr<BlobOStuff> MakeFromData(const Caps*,
                                                    ShaderCodeDictionary*,
                                                    const SkData* data);

    static std::unique_ptr<BlobOStuff> MakeFromData(const Caps*, ShaderCodeDictionary*,
                                                    const void* data, size_t size);

    sk_sp<SkData> snap1(SharedContext*) const;

    const GraphicsPipelineDesc& gpd() const { return fPipelineDesc; }
    const RenderPassDesc& rpd() const { return fRenderPassDesc; }

    bool compare(const BlobOStuff* other) const;

private:
    GraphicsPipelineDesc fPipelineDesc;
    RenderPassDesc fRenderPassDesc;
};


} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_BlobOStuff_DEFINED
