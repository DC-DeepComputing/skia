/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/dawn/DawnUtils.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtilsPriv.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/dawn/DawnQueueManager.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

namespace skgpu::graphite {

namespace ContextFactory {
std::unique_ptr<Context> MakeDawn(const DawnBackendContext& backendContext,
                                  const ContextOptions& options) {
    sk_sp<SharedContext> sharedContext = DawnSharedContext::Make(backendContext, options);
    if (!sharedContext) {
        return nullptr;
    }

    auto queueManager =
            std::make_unique<DawnQueueManager>(backendContext.fQueue, sharedContext.get());
    if (!queueManager) {
        return nullptr;
    }

    auto context = ContextCtorAccessor::MakeContext(std::move(sharedContext),
                                                    std::move(queueManager),
                                                    options);
    SkASSERT(context);
    return context;
}
} // namespace ContextFactory

bool DawnFormatIsDepthOrStencil(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::Stencil8:             [[fallthrough]];
        case wgpu::TextureFormat::Depth16Unorm:
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return true;
        default:
            return false;
    }
}

bool DawnFormatIsDepth(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::Depth16Unorm:         [[fallthrough]];
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return true;
        default:
            return false;
    }
}

bool DawnFormatIsStencil(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::Stencil8:             [[fallthrough]];
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return true;
        default:
            return false;
    }
}

wgpu::TextureFormat DawnDepthStencilFlagsToFormat(SkEnumBitMask<DepthStencilFlags> mask) {
    // TODO: Decide if we want to change this to always return a combined depth and stencil format
    // to allow more sharing of depth stencil allocations.
    if (mask == DepthStencilFlags::kDepth) {
        // If needed for workarounds or performance, Depth32Float is also available but requires 2x
        // the amount of memory.
        return wgpu::TextureFormat::Depth16Unorm;
    } else if (mask == DepthStencilFlags::kStencil) {
        return wgpu::TextureFormat::Stencil8;
    } else if (mask == DepthStencilFlags::kDepthStencil) {
        return wgpu::TextureFormat::Depth24PlusStencil8;
    }
    SkASSERT(false);
    return wgpu::TextureFormat::Undefined;
}

static bool check_shader_module([[maybe_unused]] const DawnSharedContext* sharedContext,
                                wgpu::ShaderModule* module,
                                const char* shaderText,
                                ShaderErrorHandler* errorHandler) {
    // Prior to emsdk 3.1.51 wgpu::ShaderModule::GetCompilationInfo is unimplemented.
#if defined(__EMSCRIPTEN__)                                      &&  \
        ((__EMSCRIPTEN_major__ <  3                               || \
         (__EMSCRIPTEN_major__ == 3 && __EMSCRIPTEN_minor__ <  1) || \
         (__EMSCRIPTEN_major__ == 3 && __EMSCRIPTEN_minor__ == 1 && __EMSCRIPTEN_tiny__ < 51)))
    return true;
#else
    struct Handler {
        static void Fn(WGPUCompilationInfoRequestStatus status,
                       const WGPUCompilationInfo* info,
                       void* userdata) {
            Handler* self = reinterpret_cast<Handler*>(userdata);
            SkASSERT(status == WGPUCompilationInfoRequestStatus_Success);

            // Walk the message list and check for hard errors.
            self->fSuccess = true;
            for (size_t index = 0; index < info->messageCount; ++index) {
                const WGPUCompilationMessage& entry = info->messages[index];
                if (entry.type == WGPUCompilationMessageType_Error) {
                    self->fSuccess = false;
                    break;
                }
            }

            // If we found a hard error, report the compilation messages to the error handler.
            if (!self->fSuccess) {
                std::string errors;
                for (size_t index = 0; index < info->messageCount; ++index) {
                    const WGPUCompilationMessage& entry = info->messages[index];
                    errors += "line " +
                              std::to_string(entry.lineNum) + ':' +
                              std::to_string(entry.linePos) + ' ' +
                              entry.message + '\n';
                }
                self->fErrorHandler->compileError(
                        self->fShaderText, errors.c_str(), /*shaderWasCached=*/false);
            }
        }

        const char* fShaderText;
        ShaderErrorHandler* fErrorHandler;
        bool fSuccess = false;
    };

    Handler handler;
    handler.fShaderText = shaderText;
    handler.fErrorHandler = errorHandler;
#if defined(__EMSCRIPTEN__)
    // Deprecated function.
    module->GetCompilationInfo(&Handler::Fn, &handler);
#else
    // New API.
    wgpu::FutureWaitInfo waitInfo{};
    waitInfo.future = module->GetCompilationInfo(
            wgpu::CallbackMode::WaitAnyOnly,
            [handlerPtr = &handler](wgpu::CompilationInfoRequestStatus status,
                                    const wgpu::CompilationInfo* info) {
                Handler::Fn(static_cast<WGPUCompilationInfoRequestStatus>(status),
                            reinterpret_cast<const WGPUCompilationInfo*>(info),
                            handlerPtr);
            });

    const auto& instance = static_cast<const DawnSharedContext*>(sharedContext)
                                   ->device()
                                   .GetAdapter()
                                   .GetInstance();
    [[maybe_unused]] auto status =
            instance.WaitAny(1, &waitInfo, /*timeoutNS=*/std::numeric_limits<uint64_t>::max());
    SkASSERT(status == wgpu::WaitStatus::Success);
#endif  // defined(__EMSCRIPTEN__)

    return handler.fSuccess;
#endif
}

bool DawnCompileWGSLShaderModule(const DawnSharedContext* sharedContext,
                                 const char* label,
                                 const std::string& wgsl,
                                 wgpu::ShaderModule* module,
                                 ShaderErrorHandler* errorHandler) {
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
    wgslDesc.code = wgsl.c_str();

    wgpu::ShaderModuleDescriptor desc;
    desc.nextInChain = &wgslDesc;
    if (sharedContext->caps()->setBackendLabels()) {
        desc.label = label;
    }

    *module = sharedContext->device().CreateShaderModule(&desc);

    return check_shader_module(sharedContext, module, wgsl.c_str(), errorHandler);
}

#if !defined(__EMSCRIPTEN__)
namespace ycbcrUtils {

bool descriptorIsValid(const wgpu::YCbCrVkDescriptor& desc) {
    static const wgpu::YCbCrVkDescriptor kDefaultYcbcrDescriptor = {};
    return !descriptorsAreEquivalent(desc, kDefaultYcbcrDescriptor);
}

uint32_t nonformatInfoAsUint32(const wgpu::YCbCrVkDescriptor& ycbcrDesc) {
    static_assert(kComponentAShift + kComponentBits <= 32);
    SkASSERT(ycbcrDesc.vkYCbCrModel                          < (1u << kYcbcrModelBits    ));
    SkASSERT(ycbcrDesc.vkYCbCrRange                          < (1u << kYcbcrRangeBits    ));
    SkASSERT(ycbcrDesc.vkXChromaOffset                       < (1u << kXChromaOffsetBits ));
    SkASSERT(ycbcrDesc.vkYChromaOffset                       < (1u << kYChromaOffsetBits ));
    SkASSERT(static_cast<uint32_t>(ycbcrDesc.vkChromaFilter) < (1u << kChromaFilterBits  ));
    SkASSERT(ycbcrDesc.vkComponentSwizzleRed                 < (1u << kComponentBits     ));
    SkASSERT(ycbcrDesc.vkComponentSwizzleGreen               < (1u << kComponentBits     ));
    SkASSERT(ycbcrDesc.vkComponentSwizzleBlue                < (1u << kComponentBits     ));
    SkASSERT(ycbcrDesc.vkComponentSwizzleAlpha               < (1u << kComponentBits     ));
    SkASSERT(static_cast<uint32_t>(ycbcrDesc.forceExplicitReconstruction)
             < (1u << kForceExplicitReconBits));

    return (((uint32_t)(usesExternalFormat(ycbcrDesc)         ) << kUsesExternalFormatShift) |
            ((uint32_t)(ycbcrDesc.vkYCbCrModel                ) << kYcbcrModelShift        ) |
            ((uint32_t)(ycbcrDesc.vkYCbCrRange                ) << kYcbcrRangeShift        ) |
            ((uint32_t)(ycbcrDesc.vkXChromaOffset             ) << kXChromaOffsetShift     ) |
            ((uint32_t)(ycbcrDesc.vkYChromaOffset             ) << kYChromaOffsetShift     ) |
            ((uint32_t)(ycbcrDesc.vkChromaFilter              ) << kChromaFilterShift      ) |
            ((uint32_t)(ycbcrDesc.forceExplicitReconstruction ) << kForceExplicitReconShift) |
            ((uint32_t)(ycbcrDesc.vkComponentSwizzleRed       ) << kComponentRShift        ) |
            ((uint32_t)(ycbcrDesc.vkComponentSwizzleGreen     ) << kComponentGShift        ) |
            ((uint32_t)(ycbcrDesc.vkComponentSwizzleBlue      ) << kComponentBShift        ) |
            ((uint32_t)(ycbcrDesc.vkComponentSwizzleAlpha     ) << kComponentAShift        ));
}

bool usesExternalFormat(const wgpu::YCbCrVkDescriptor& desc) {
    return desc.vkFormat == 0 && desc.externalFormat != 0;
}

int numInt32sNeeded(const wgpu::YCbCrVkDescriptor& desc) {
    return usesExternalFormat(desc) ? kIntsNeededExternalFormat : kIntsNeededKnownFormat;
}

wgpu::YCbCrVkDescriptor getDescriptorFromSamplerDesc(const SamplerDesc& samplerDesc) {
    wgpu::YCbCrVkDescriptor desc;
    if (samplerDesc.isImmutable()) {
        uint32_t nonFormatInfo =
                (uint32_t)(samplerDesc.desc() >> SamplerDesc::kImmutableSamplerInfoShift);

        desc.vkFormat = samplerDesc.usesExternalFormat() ? 0 : samplerDesc.format();
        desc.vkYCbCrModel = (nonFormatInfo & kYcbcrModelMask) >> kYcbcrModelShift;
        desc.vkYCbCrRange = (nonFormatInfo & kYcbcrRangeMask) >> kYcbcrRangeShift;
        desc.vkComponentSwizzleRed = (nonFormatInfo & kComponentRMask) >> kComponentRShift;
        desc.vkComponentSwizzleGreen = (nonFormatInfo & kComponentGMask) >> kComponentGShift;
        desc.vkComponentSwizzleBlue = (nonFormatInfo & kComponentBMask) >> kComponentBShift;
        desc.vkComponentSwizzleAlpha = (nonFormatInfo & kComponentAMask) >> kComponentAShift;
        desc.vkXChromaOffset = (nonFormatInfo & kXChromaOffsetMask) >> kXChromaOffsetShift;
        desc.vkYChromaOffset = (nonFormatInfo & kYChromaOffsetMask) >> kYChromaOffsetShift;
        desc.vkChromaFilter =
                (wgpu::FilterMode)((nonFormatInfo & kChromaFilterMask) >> kChromaFilterShift);
        desc.forceExplicitReconstruction =
                (nonFormatInfo & kForceExplicitReconMask) >> kForceExplicitReconShift;
        desc.externalFormat = samplerDesc.usesExternalFormat() ?
                (uint64_t)(samplerDesc.externalFormatMSBs()) << 32 | samplerDesc.format() : 0;
    }
    return desc;
}

} // namespace ycbcrUtils
#endif // !defined(__EMSCRIPTEN__)

} // namespace skgpu::graphite
