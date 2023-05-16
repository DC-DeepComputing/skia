/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/vk/VulkanQueueManager.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/utils/SkShaderUtils.h"


namespace skgpu::graphite::ContextFactory {

std::unique_ptr<Context> MakeVulkan(const VulkanBackendContext& backendContext,
                                    const ContextOptions& options) {
    sk_sp<SharedContext> sharedContext = VulkanSharedContext::Make(backendContext, options);
    if (!sharedContext) {
        return nullptr;
    }

    std::unique_ptr<QueueManager> queueManager(new VulkanQueueManager(backendContext.fQueue,
                                                                      sharedContext.get()));
    if (!queueManager) {
        return nullptr;
    }

    return ContextCtorAccessor::MakeContext(std::move(sharedContext),
                                            std::move(queueManager),
                                            options);
}
} // namespace skgpu::graphite::ContextFactory

namespace skgpu::graphite {

// Print the source code for all shaders generated.
#ifdef SK_PRINT_SKSL_SHADERS
static constexpr bool gPrintSKSL  = true;
#else
static constexpr bool gPrintSKSL  = false;
#endif

bool SkSLToSPIRV(SkSL::Compiler* compiler,
                 const std::string& sksl,
                 SkSL::ProgramKind programKind,
                 const SkSL::ProgramSettings& settings,
                 std::string* spirv,
                 SkSL::Program::Interface* outInterface,
                 ShaderErrorHandler* errorHandler) {
#ifdef SK_DEBUG
    std::string src = SkShaderUtils::PrettyPrint(sksl);
#else
    const std::string& src = sksl;
#endif
    std::unique_ptr<SkSL::Program> program = compiler->convertProgram(programKind,
                                                                      src,
                                                                      settings);
    if (!program || !compiler->toSPIRV(*program, spirv)) {
        errorHandler->compileError(src.c_str(), compiler->errorText().c_str());
        return false;
    }

    if (gPrintSKSL) {
        SkShaderUtils::PrintShaderBanner(programKind);
        SkDebugf("SKSL:\n");
        SkShaderUtils::PrintLineByLine(SkShaderUtils::PrettyPrint(sksl));
    }

    *outInterface = program->fInterface;
    return true;
}

VkShaderModule createVulkanShaderModule(const VulkanSharedContext* context,
                                        const std::string& spirv,
                                        VkShaderStageFlagBits stage) {
    // TODO add trace event
    VkShaderModuleCreateInfo moduleCreateInfo;
    memset(&moduleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = nullptr;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = spirv.size();
    moduleCreateInfo.pCode = (const uint32_t*)spirv.c_str();

    VkShaderModule shaderModule;
    VkResult result;
    VULKAN_CALL_RESULT(context->interface(),
                       result,
                       CreateShaderModule(context->device(),
                                          &moduleCreateInfo,
                                          /*const VkAllocationCallbacks*=*/nullptr,
                                          &shaderModule));
    if (result != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    return shaderModule;
}

} // namespace skgpu::graphite
