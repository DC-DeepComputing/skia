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

class BlobOStuff {
public:
    static const int kCurrent_Version = 1;

    BlobOStuff(const GraphicsPipelineDesc& pipelineDesc, const RenderPassDesc& renderPassDesc)
               : fPipelineDesc(pipelineDesc)
               , fRenderPassDesc(renderPassDesc) {}

    static std::unique_ptr<BlobOStuff> MakeFromStream(SkStream* stream,
                                                      ShaderCodeDictionary*,
                                                      const Caps* caps);

    static std::unique_ptr<BlobOStuff> MakeFromData(const SkData* data,
                                                    ShaderCodeDictionary*,
                                                    const Caps* caps);

    static std::unique_ptr<BlobOStuff> MakeFromData(const void* data, size_t size,
                                                    ShaderCodeDictionary*,
                                                    const Caps* caps);

    sk_sp<SkData> snap1(SharedContext*) const;

    const GraphicsPipelineDesc& gpd() const { return fPipelineDesc; }
    const RenderPassDesc& rpd() const { return fRenderPassDesc; }

    bool compare(const BlobOStuff* other) const;

private:
    // returns true on success; false on failure
    [[nodiscard]] bool serialize(SharedContext*, SkWStream*) const;
    static std::unique_ptr<BlobOStuff> Deserialize(SkStream* stream,
                                                   ShaderCodeDictionary*,
                                                   const Caps*);

    GraphicsPipelineDesc fPipelineDesc;
    RenderPassDesc fRenderPassDesc;
};


} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_BlobOStuff_DEFINED
