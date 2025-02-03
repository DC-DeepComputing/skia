/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_DawnUtilsPriv_DEFINED
#define skgpu_DawnUtilsPriv_DEFINED

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/graphite/dawn/DawnTypes.h"
#include "src/gpu/SkSLToBackend.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/sksl/codegen/SkSLWGSLCodeGenerator.h"
#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

namespace SkSL {

enum class ProgramKind : int8_t;
struct ProgramInterface;
struct ProgramSettings;
struct ShaderCaps;

}  // namespace SkSL

namespace skgpu {

class BackendTexture;
class ShaderErrorHandler;

inline bool SkSLToWGSL(const SkSL::ShaderCaps* caps,
                       const std::string& sksl,
                       SkSL::ProgramKind programKind,
                       const SkSL::ProgramSettings& settings,
                       std::string* wgsl,
                       SkSL::ProgramInterface* outInterface,
                       ShaderErrorHandler* errorHandler) {
    return SkSLToBackend(caps, &SkSL::ToWGSL, "WGSL",
                         sksl, programKind, settings, wgsl, outInterface, errorHandler);
}

namespace graphite {

class DawnSharedContext;

size_t DawnFormatBytesPerBlock(wgpu::TextureFormat format);

SkTextureCompressionType DawnFormatToCompressionType(wgpu::TextureFormat format);

uint32_t DawnFormatChannels(wgpu::TextureFormat format);


bool DawnFormatIsDepthOrStencil(wgpu::TextureFormat);
bool DawnFormatIsDepth(wgpu::TextureFormat);
bool DawnFormatIsStencil(wgpu::TextureFormat);

wgpu::TextureFormat DawnDepthStencilFlagsToFormat(SkEnumBitMask<DepthStencilFlags>);

bool DawnCompileWGSLShaderModule(const DawnSharedContext* sharedContext,
                                 const char* label,
                                 const std::string& wgsl,
                                 wgpu::ShaderModule* module,
                                 ShaderErrorHandler*);

#if !defined(__EMSCRIPTEN__)

bool DawnDescriptorIsValid(const wgpu::YCbCrVkDescriptor&);

bool DawnDescriptorUsesExternalFormat(const wgpu::YCbCrVkDescriptor&);

bool DawnDescriptorsAreEquivalent(const wgpu::YCbCrVkDescriptor&, const wgpu::YCbCrVkDescriptor&);

ImmutableSamplerInfo DawnDescriptorToImmutableSamplerInfo(const wgpu::YCbCrVkDescriptor&);
wgpu::YCbCrVkDescriptor DawnDescriptorFromImmutableSamplerInfo(ImmutableSamplerInfo);

#endif // !defined(__EMSCRIPTEN__)


namespace BackendTextures {

WGPUTexture GetDawnTexturePtr(const BackendTexture&);
WGPUTextureView GetDawnTextureViewPtr(const BackendTexture&);

}  // namespace BackendTextures

}  // namespace graphite
}  // namespace skgpu

#endif // skgpu_DawnUtilsPriv_DEFINED
