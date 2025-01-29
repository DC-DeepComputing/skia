/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_SerializationUtils_DEFINED
#define skgpu_graphite_precompile_SerializationUtils_DEFINED

#include "include/core/SkTypes.h"

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
namespace SerializationUtils {

[[nodiscard]] bool Serialize(ShaderCodeDictionary*,
                             const GraphicsPipelineDesc&,
                             const RenderPassDesc&,
                             SkWStream*);
[[nodiscard]] bool Deserialize(const Caps*,
                               ShaderCodeDictionary*,
                               SkStream*,
                               GraphicsPipelineDesc*,
                               RenderPassDesc*);

} // SerializationUtils

} // skgpu::graphite

#endif // skgpu_graphite_precompile_SerializationUtils_DEFINED
