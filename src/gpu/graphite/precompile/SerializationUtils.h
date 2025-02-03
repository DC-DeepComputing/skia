/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_SerializationUtils_DEFINED
#define skgpu_graphite_precompile_SerializationUtils_DEFINED

#include "include/core/SkRefCnt.h"

class SkData;
class SkStream;
class SkWStream;

namespace skgpu::graphite {

class Caps;
class GraphicsPipelineDesc;
class RendererProvider;
struct RenderPassDesc;
class ShaderCodeDictionary;

// These are the top-level entry points to serialize Pipeline data for the Android-style
// Precompilation API
[[nodiscard]] bool SerializePipelineDesc(ShaderCodeDictionary*,
                                         SkWStream*,
                                         const GraphicsPipelineDesc&,
                                         const RenderPassDesc&);
[[nodiscard]] bool DeserializePipelineDesc(const Caps*,
                                           ShaderCodeDictionary*,
                                           SkStream*,
                                           GraphicsPipelineDesc*,
                                           RenderPassDesc*);

[[nodiscard]] sk_sp<SkData> Snap1(ShaderCodeDictionary*,
                                  const GraphicsPipelineDesc&,
                                  const RenderPassDesc&);

[[nodiscard]] bool MakeFromData1(const Caps*,
                                 ShaderCodeDictionary*,
                                 const SkData*,
                                 GraphicsPipelineDesc* pipelineDesc,
                                 RenderPassDesc* renderPassDesc);

#if defined(SK_DEBUG)
void Dump1(const char* label,
           ShaderCodeDictionary*,
           const GraphicsPipelineDesc&,
           const RenderPassDesc&);

[[nodiscard]] bool Compare1(const GraphicsPipelineDesc& a1, const RenderPassDesc& b1,
                            const GraphicsPipelineDesc& a2, const RenderPassDesc& b2);
#endif

} // skgpu::graphite

#endif // skgpu_graphite_precompile_SerializationUtils_DEFINED
