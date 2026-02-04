/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/vk/VulkanPreferredFeatures.h"
#include "include/private/base/SkMacros.h"
#include "src/gpu/vk/VulkanInterface.h"
#include "tools/gpu/vk/VkTestMemoryAllocator.h"
#include "tools/gpu/vk/VkTestUtils.h"

#ifdef SK_VULKAN

#ifndef SK_GPU_TOOLS_VK_LIBRARY_NAME
    #if defined _WIN32
        #define SK_GPU_TOOLS_VK_LIBRARY_NAME vulkan-1.dll
    #elif defined SK_BUILD_FOR_MAC
        #define SK_GPU_TOOLS_VK_LIBRARY_NAME libvk_swiftshader.dylib
    #else
        #define SK_GPU_TOOLS_VK_LIBRARY_NAME        libvulkan.so
        #define SK_GPU_TOOLS_VK_LIBRARY_NAME_BACKUP libvulkan.so.1
    #endif
#endif

#include <algorithm>

#if defined(__GLIBC__)
#include <execinfo.h>
#endif
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "src/base/SkAutoMalloc.h"
#include "tools/library/LoadDynamicLibrary.h"

#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
#include <sanitizer/lsan_interface.h>
#endif

using namespace skia_private;

namespace sk_gpu_test {

bool LoadVkLibraryAndGetProcAddrFuncs(PFN_vkGetInstanceProcAddr* instProc) {
    static void* vkLib = nullptr;
    static PFN_vkGetInstanceProcAddr localInstProc = nullptr;
    if (!vkLib) {
        vkLib = SkLoadDynamicLibrary(SK_MACRO_STRINGIFY(SK_GPU_TOOLS_VK_LIBRARY_NAME));
        if (!vkLib) {
            // vulkaninfo tries to load the library from two places, so we do as well
            // https://github.com/KhronosGroup/Vulkan-Tools/blob/078d44e4664b7efa0b6c96ebced1995c4425d57a/vulkaninfo/vulkaninfo.h#L249
#ifdef SK_GPU_TOOLS_VK_LIBRARY_NAME_BACKUP
            vkLib = SkLoadDynamicLibrary(SK_MACRO_STRINGIFY(SK_GPU_TOOLS_VK_LIBRARY_NAME_BACKUP));
            if (!vkLib) {
                return false;
            }
#else
            return false;
#endif
        }
        localInstProc = (PFN_vkGetInstanceProcAddr) SkGetProcedureAddress(vkLib,
                                                                          "vkGetInstanceProcAddr");
    }
    if (!localInstProc) {
        return false;
    }
    *instProc = localInstProc;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Helper code to set up Vulkan context objects

#if defined SK_ENABLE_VK_LAYERS
static uint32_t remove_patch_version(uint32_t specVersion) {
    return (specVersion >> 12) << 12;
}

static void print_backtrace() {
#if defined(__GLIBC__)
    void* stack[64];
    int count = backtrace(stack, std::size(stack));
    backtrace_symbols_fd(stack, count, 2);
#else
    // Please add implementations for other platforms.
#endif
}

VKAPI_ATTR VkBool32 VKAPI_CALL
DebugUtilsMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                    void* userData) {
    // VUID-VkDebugUtilsMessengerCallbackDataEXT-pMessage-parameter
    // pMessage must be a null-terminated UTF-8 string
    SkASSERT(callbackData->pMessage != nullptr);

    static constexpr const char* kSkippedMessages[] = {
            "Nothing for now, this string works around msvc bug with empty array",
    };

    // See if it's an issue we are aware of and don't want to be spammed about.
    // Always report the debug message if message ID is missing
    if (callbackData->pMessageIdName != nullptr) {
        for (const char* skipped : kSkippedMessages) {
            if (strstr(callbackData->pMessageIdName, skipped) != nullptr) {
                return VK_FALSE;
            }
        }
    }

    bool printStackTrace = true;
    bool fail = false;

    const char* severity = "message";
    if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
        severity = "error";
        fail = true;
    } else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
        severity = "warning";
    } else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) {
        severity = "info";
        printStackTrace = false;
    }

    std::string type;
    if ((messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) != 0) {
        type += " <general>";
    }
    if ((messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0) {
        type += " <validation>";
    }
    if ((messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0) {
        type += " <performance>";
    }

    SkDebugf("Vulkan %s%s [%s]: %s\n",
             severity,
             type.c_str(),
             callbackData->pMessageIdName ? callbackData->pMessageIdName : "<no id>",
             callbackData->pMessage);

    if (printStackTrace) {
        print_backtrace();
    }

    if (fail) {
        SkDEBUGFAIL("Vulkan debug layer error");
    }

    return VK_FALSE;
}
#endif // SK_ENABLE_VK_LAYERS

#define ACQUIRE_VK_INST_PROC_LOCAL(name, instance)                                 \
    PFN_vk##name grVk##name =                                                      \
        reinterpret_cast<PFN_vk##name>(getInstProc(instance, "vk" #name));         \
    do {                                                                           \
        if (grVk##name == nullptr) {                                               \
            SkDebugf("Function ptr for vk%s could not be acquired\n", #name);      \
            return false;                                                          \
        }                                                                          \
    } while (0)

#define ACQUIRE_VK_INST_PROC_LOCAL_NOCHECK(name, instance)                         \
    PFN_vk##name grVk##name = reinterpret_cast<PFN_vk##name>(getInstProc(instance, "vk" #name))

// Returns the index into layers array for the layer we want. Returns -1 if not supported.
static bool include_device_extension(const char* extName) {
    const char* kTestDeviceExtensions[] = {
        VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME,
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_DEVICE_FAULT_EXTENSION_NAME,
        VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME,
        // Extensions currently only used by Ganesh:
        VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME,
        VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME,
    };

    for (size_t i = 0; i < std::size(kTestDeviceExtensions); ++i) {
        if (!strcmp(extName, kTestDeviceExtensions[i])) {
            return true;
        }
    }
    return false;
}

static bool include_instance_extension(const char* extName) {
    const char* kTestInstanceExtensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_EXT_LAYER_SETTINGS_EXTENSION_NAME,
        // Platform-specific extensions
        // The name macros like we use above are all defined in platform-specific vulkan headers, which
        // we currently do not include since they are a bit of a pain (e.g. windows headers require
        // including <windows.h>, which causes all sorts of annoyances and problems). Instead, just list
        // the strings these macros are defined to. This is unlikely to cause long-term issues because
        // it is *highly* unlikely that the strings connected to the name macros will ever change.
        "VK_KHR_win32_surface",
        "VK_KHR_xcb_surface",
        "VK_KHR_android_surface",
    };

    for (size_t i = 0; i < std::size(kTestInstanceExtensions); ++i) {
        if (!strcmp(extName, kTestInstanceExtensions[i])) {
            return true;
        }
    }
    return false;
}

#ifdef SK_ENABLE_VK_LAYERS
// Checks a list of availableLayers for debug layers we want to use (kDebugLayerNames), appending
// the layer to a passed-in layersToUse if it is present and can be used with our Vulkan version.
static void append_debug_layers(const VkLayerProperties* availableLayers,
                                const uint32_t availLayerCount,
                                const uint32_t majorSpecVersion,
                                TArray<VkLayerProperties>* layersToUse,
                                TArray<const char*>* layerNames) {
    // Comment out or include different debugging layers by modifying the array below:
    const char* kDebugLayerNames[] = {
        "VK_LAYER_KHRONOS_validation",
        // Not included in standard_validation:
        // "VK_LAYER_LUNARG_api_dump",
        // "VK_LAYER_LUNARG_vktrace",
        // "VK_LAYER_LUNARG_screenshot",
    };

    // Check for each desired debug layer within the list of available layers. If we encounter a
    // match, verify that the layer was authored against a version >= the major Vulkan version we
    // are using before appending it to the list of layers to use. This ensures the validation
    // layers will have all the expected API entry points in order to intercept Vulkan calls.
    for (size_t i = 0; i < std::size(kDebugLayerNames); ++i) {
        for (size_t j = 0; j < availLayerCount; ++j) {
            if (!strcmp(kDebugLayerNames[i], availableLayers[j].layerName) &&
                majorSpecVersion <= remove_patch_version(availableLayers[j].specVersion)) {
                layersToUse->push_back(availableLayers[i]);
                layerNames->push_back(availableLayers[i].layerName);
            }
        }
    }
}
// Determine available instance layers and append any that should be included (as defined by
// kTestInstanceExtensions) to our list of instance layers.
bool append_available_inst_debug_layers(PFN_vkGetInstanceProcAddr getInstProc,
                                        uint32_t majorSpecVersion,
                                        TArray<VkLayerProperties>* instLayers,
                                        TArray<const char*>* instLayerNames) {
    ACQUIRE_VK_INST_PROC_LOCAL_NOCHECK(EnumerateInstanceLayerProperties, VK_NULL_HANDLE);
    if (!grVkEnumerateInstanceLayerProperties) {
        return false;
    }

    VkResult result;
    uint32_t availLayerCount = 0;
    result = grVkEnumerateInstanceLayerProperties(&availLayerCount, /*pProperties=*/nullptr);
    if (result != VK_SUCCESS) {
        return false;
    }

    VkLayerProperties* availDevLayers = new VkLayerProperties[availLayerCount];
    result = grVkEnumerateInstanceLayerProperties(&availLayerCount, availDevLayers);
    if (result != VK_SUCCESS) {
        delete[] availDevLayers;
        return false;
    }

    // Add any debug-specific layers used in testing to the instance layers
    append_debug_layers(
            availDevLayers, availLayerCount, majorSpecVersion, instLayers, instLayerNames);

    delete[] availDevLayers;
    return true;
}
#endif // SK_ENABLE_VK_LAYERS

// An empty / null layerName means we are checking extensions provided by the Vulkan version or
// implicitly enabled layers. An actual layer name checks instance extensions provided by that layer
static bool append_available_inst_extensions(PFN_vkGetInstanceProcAddr getInstProc,
                                             const char* layerName,
                                             TArray<VkExtensionProperties>* instExtensions,
                                             std::vector<const char*>* instExtNames) {
    ACQUIRE_VK_INST_PROC_LOCAL_NOCHECK(EnumerateInstanceExtensionProperties, VK_NULL_HANDLE);
    if (!grVkEnumerateInstanceExtensionProperties) {
        return false;
    }

    VkResult result;
    uint32_t availExtCount = 0;
    result = grVkEnumerateInstanceExtensionProperties(layerName,
                                                      &availExtCount,
                                                      /*pProperties=*/nullptr);
    if (result != VK_SUCCESS) {
        return false;
    }
    VkExtensionProperties* availExtensions = new VkExtensionProperties[availExtCount];
    result = grVkEnumerateInstanceExtensionProperties(layerName, &availExtCount, availExtensions);
    if (result != VK_SUCCESS) {
        delete[] availExtensions;
        return false;
    }

    for (uint32_t i = 0; i < availExtCount; ++i) {
        if (include_instance_extension(availExtensions[i].extensionName)) {
            instExtensions->push_back() = availExtensions[i];
            instExtNames->push_back(availExtensions[i].extensionName);
        }
    }

    delete[] availExtensions;
    return true;
}

// Helper to initialize a list of instance extensions and layers to use based upon those available
static bool gather_test_inst_extensions_and_layers(
        PFN_vkGetInstanceProcAddr getInstProc,
        uint32_t majorSpecVersion,
        TArray<VkLayerProperties>* instLayers,
        TArray<const char*>* instLayerNames,
        TArray<VkExtensionProperties>* instExtensions,
        std::vector<const char*>* instExtNames) {
    SkASSERT(getInstProc);

#ifdef SK_ENABLE_VK_LAYERS
    if (!append_available_inst_debug_layers(
                getInstProc, majorSpecVersion, instLayers, instLayerNames)) {
        return false;
    }
#endif // SK_ENABLE_VK_LAYERS

    // Instance extensions from Vulkan implementation + implicitly enabled layers:
    if (!append_available_inst_extensions(
                getInstProc, /*layerName=*/"", instExtensions, instExtNames)) {
        return false;
    }

    // Instance extensions from explicitly enabled layers:
    uint32_t enabledLayerCount = instLayers->size();
    for (uint32_t i = 0; i < enabledLayerCount; ++i) {
        if (!append_available_inst_extensions(getInstProc,
                                              (*instLayers)[i].layerName,
                                              instExtensions,
                                              instExtNames)) {
            return false;
        }
    }

    return true;
}

#define GET_PROC_LOCAL(F, inst, device) PFN_vk ## F F = (PFN_vk ## F) getProc("vk" #F, inst, device)

#ifdef SK_ENABLE_VK_LAYERS
// Determine available device layers and append any that should be included (as defined by
// kTestDeviceExtensions) to our list of deviceLayers.
bool append_available_device_debug_layers(const skgpu::VulkanGetProc& getProc,
                                          VkInstance inst,
                                          VkPhysicalDevice physDev,
                                          uint32_t majorSpecVersion,
                                          TArray<VkLayerProperties>* devLayers,
                                          TArray<const char*>* devLayerNames) {
    GET_PROC_LOCAL(EnumerateDeviceLayerProperties, inst, /*device=*/VK_NULL_HANDLE);
    if (!EnumerateDeviceLayerProperties) {
        return false;
    }

    VkResult result;
    uint32_t availLayerCount = 0;
    result = EnumerateDeviceLayerProperties(physDev, &availLayerCount, /*pProperties=*/nullptr);
    if (result != VK_SUCCESS) {
        return false;
    }
    VkLayerProperties* availLayers = new VkLayerProperties[availLayerCount];
    result = EnumerateDeviceLayerProperties(physDev, &availLayerCount, availLayers);
    if (result != VK_SUCCESS) {
        delete[] availLayers;
        return false;
    }

    append_debug_layers(availLayers, availLayerCount, majorSpecVersion, devLayers, devLayerNames);

    delete[] availLayers;
    return true;
}
#endif // SK_ENABLE_VK_LAYERS

// An empty / null layerName means we are checking extensions provided by the Vulkan version or
// implicitly enabled layers. An actual layer name checks device extensions provided by that layer.
static bool append_available_device_extensions(const skgpu::VulkanGetProc& getProc,
                                               VkInstance inst,
                                               VkPhysicalDevice physDev,
                                               const char* layerName,
                                               TArray<VkExtensionProperties>* devExtensions,
                                               std::vector<const char*>* devExtNames) {
    GET_PROC_LOCAL(EnumerateDeviceExtensionProperties, inst, /*device=*/VK_NULL_HANDLE);
    if (!EnumerateDeviceExtensionProperties) {
        return false;
    }

    VkResult result;
    uint32_t availExtCount = 0;
    result = EnumerateDeviceExtensionProperties(physDev,
                                                layerName,
                                                &availExtCount,
                                                /*pProperties=*/nullptr);
    if (result != VK_SUCCESS) {
        return false;
    }
    VkExtensionProperties* availExtensions = new VkExtensionProperties[availExtCount];
    result = EnumerateDeviceExtensionProperties(physDev,
                                                layerName,
                                                &availExtCount,
                                                availExtensions);
    if (result != VK_SUCCESS) {
        delete[] availExtensions;
        return false;
    }

    for (uint32_t i = 0; i < availExtCount; ++i) {
        if (include_device_extension(availExtensions[i].extensionName)) {
            devExtensions->push_back() = availExtensions[i];
            devExtNames->push_back(availExtensions[i].extensionName);
        }
    }
    delete[] availExtensions;

    return true;
}

// Helper to initialize a list of device extensions and layers to use based upon those available
static bool gather_test_device_extensions_and_layers(
        const skgpu::VulkanGetProc& getProc,
        uint32_t majorSpecVersion,
        VkInstance inst,
        VkPhysicalDevice physDev,
        TArray<VkLayerProperties>* devLayers,
        TArray<const char*>* devLayerNames,
        TArray<VkExtensionProperties>* devExtensions,
        std::vector<const char*>* devExtNames) {
#ifdef SK_ENABLE_VK_LAYERS
    if (!append_available_device_debug_layers(
                getProc, inst, physDev, majorSpecVersion, devLayers, devLayerNames)) {
        return false;
    }
#endif

    // Device extensions from Vulkan implementation + implicitly enabled layers:
    if (!append_available_device_extensions(
                getProc, inst, physDev, /*layerName=*/"", devExtensions, devExtNames)) {
        return false;
    }

    // Device extensions from explicitly enabled layers:
    uint32_t enabledLayerCount = devLayers->size();
    for (uint32_t i = 0; i < enabledLayerCount; ++i) {
        if (!append_available_device_extensions(
                getProc, inst, physDev, (*devLayers)[i].layerName, devExtensions, devExtNames)) {
            return false;
        }
    }

    return true;
}

#define ACQUIRE_VK_INST_PROC_NOCHECK(name, instance) \
    PFN_vk##name grVk##name = reinterpret_cast<PFN_vk##name>(getInstProc(instance, "vk" #name))

#define ACQUIRE_VK_INST_PROC(name, instance)                                                     \
    PFN_vk##name grVk##name = reinterpret_cast<PFN_vk##name>(getInstProc(instance, "vk" #name)); \
    do {                                                                                         \
        if (grVk##name == nullptr) {                                                             \
            SkDebugf("Function ptr for vk%s could not be acquired\n", #name);                    \
            if (inst != VK_NULL_HANDLE) {                                                        \
                destroy_instance(getInstProc, inst, debugMessenger, hasDebugExt);          \
            }                                                                                    \
            return false;                                                                        \
        }                                                                                        \
    } while (0)

#define ACQUIRE_VK_PROC_NOCHECK(name, instance, device) \
    PFN_vk##name grVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device))

#define ACQUIRE_VK_PROC(name, instance, device)                                         \
    PFN_vk##name grVk##name =                                                           \
            reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device));      \
    do {                                                                                \
        if (grVk##name == nullptr) {                                                    \
            SkDebugf("Function ptr for vk%s could not be acquired\n", #name);           \
            if (inst != VK_NULL_HANDLE) {                                               \
                destroy_instance(getInstProc, inst, debugMessenger, hasDebugExt); \
            }                                                                           \
            return false;                                                               \
        }                                                                               \
    } while (0)

#define ACQUIRE_VK_PROC_LOCAL(name, instance, device)                              \
    PFN_vk##name grVk##name =                                                      \
            reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device)); \
    do {                                                                           \
        if (grVk##name == nullptr) {                                               \
            SkDebugf("Function ptr for vk%s could not be acquired\n", #name);      \
            return false;                                                          \
        }                                                                          \
    } while (0)

static void destroy_instance(PFN_vkGetInstanceProcAddr getInstProc,
                             VkInstance inst,
                             VkDebugUtilsMessengerEXT* debugMessenger,
                             bool hasDebugExt) {
    if (hasDebugExt && *debugMessenger != VK_NULL_HANDLE) {
        ACQUIRE_VK_INST_PROC_LOCAL_NOCHECK(DestroyDebugUtilsMessengerEXT, inst);
        grVkDestroyDebugUtilsMessengerEXT(inst, *debugMessenger, /*pAllocator=*/nullptr);
        *debugMessenger = VK_NULL_HANDLE;
    }
    ACQUIRE_VK_INST_PROC_LOCAL_NOCHECK(DestroyInstance, inst);
    grVkDestroyInstance(inst, nullptr);
}

// For convenience, define a wrapper around destroy_instance(...) which returns false. This can be
// used whenever a failure leading to instance destruction should lead to a return value of false.
static bool destroy_instance_return_false(PFN_vkGetInstanceProcAddr getInstProc,
                                          VkInstance inst,
                                          VkDebugUtilsMessengerEXT* debugMessenger,
                                          bool hasDebugExt) {
    destroy_instance(getInstProc, inst, debugMessenger, hasDebugExt);
    return false;
}

static bool setup_features(const skgpu::VulkanGetProc& getProc,
                           VkInstance inst,
                           VkPhysicalDevice physDev,
                           uint32_t physDeviceVersion,
                           const TArray<VkExtensionProperties>& devExtensions,
                           TestVkFeatures& testVkFeatures,
                           bool isProtected) {
    SkASSERT(physDeviceVersion >= VK_API_VERSION_1_1);

    testVkFeatures.deviceFeatures = {};
    testVkFeatures.deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    // Add any features that are needed by tests before calling skiaFeatures->addFeaturesToQuery.

    void** tailPNext = &testVkFeatures.deviceFeatures.pNext;

    // If |isProtected| is given, attach that first
    testVkFeatures.protectedMemoryFeatures = {};
    if (isProtected) {
        testVkFeatures.protectedMemoryFeatures.sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES;
        *tailPNext = &testVkFeatures.protectedMemoryFeatures;
        tailPNext = &testVkFeatures.protectedMemoryFeatures.pNext;
    }

    testVkFeatures.skiaFeatures.addFeaturesToQuery(
            devExtensions.begin(), devExtensions.size(), testVkFeatures.deviceFeatures);

    ACQUIRE_VK_PROC_LOCAL(GetPhysicalDeviceFeatures2, inst, VK_NULL_HANDLE);
    grVkGetPhysicalDeviceFeatures2(physDev, &testVkFeatures.deviceFeatures);

    // Robustness has adverse effect on performance on a few GPUs, and besides we can't depend on it
    // on all platforms.
    testVkFeatures.deviceFeatures.features.robustBufferAccess = VK_FALSE;

    // If we want to disable any extension features do so here.

    if (isProtected) {
        if (!testVkFeatures.protectedMemoryFeatures.protectedMemory) {
            return false;
        }
    }
    return true;
}

static uint32_t get_instance_version(PFN_vkGetInstanceProcAddr getInstProc, uint32_t apiVersion) {
    ACQUIRE_VK_INST_PROC_LOCAL_NOCHECK(EnumerateInstanceVersion, VK_NULL_HANDLE);
    // Vulkan 1.1 is required, so vkEnumerateInstanceVersion should always be available.
    SkASSERT(grVkEnumerateInstanceVersion != nullptr);

    VkResult result;
    uint32_t instanceVersion = 0;
    result = grVkEnumerateInstanceVersion(&instanceVersion);
    if (result != VK_SUCCESS) {
        SkDebugf("Failed to enumerate instance version. Err: %d\n", result);
        return false;
    }
    SkASSERT(instanceVersion >= VK_API_VERSION_1_1);

    return std::min(instanceVersion, apiVersion);
}

#if defined SK_ENABLE_VK_LAYERS
static void populate_layer_settings(VkLayerSettingsCreateInfoEXT* createInfo,
                                    VkLayerSettingEXT* layerSettings,
                                    uint8_t layerSettingCount) {
    // Fine grain control of validation layer features
    const VkBool32 coreValidation = VK_TRUE;
    // Syncval is disabled for now, but would be useful to enable eventually.
    const VkBool32 syncValidation = VK_FALSE;
    const VkBool32 threadSafetyValidation = VK_TRUE;
    // Shader validation could be useful (previously broken on Android, might already be fixed:
    // http://anglebug.com/42265520).
    const VkBool32 shaderValidation = VK_FALSE;
    // If syncval is enabled, submit time validation could stay disabled due to performance issues:
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7285
    const VkBool32 syncValSubmitTime = VK_FALSE;
    // Extra properties in syncval make it easier to filter the messages.
    const VkBool32 syncValExtraMessage = VK_TRUE;

    // Utility variables for conciseness
    static const char* kLayerName = "VK_LAYER_KHRONOS_validation";
    static const uint8_t kValCount = 1;
    static const VkLayerSettingTypeEXT kType = VK_LAYER_SETTING_TYPE_BOOL32_EXT;

    layerSettings[0] = { kLayerName, "validate_core", kType, kValCount, &coreValidation };
    layerSettings[1] = { kLayerName, "validate_sync", kType, kValCount, &syncValidation };
    layerSettings[2] = { kLayerName, "thread_safety", kType, kValCount, &threadSafetyValidation };
    layerSettings[3] = { kLayerName, "check_shaders", kType, kValCount, &shaderValidation };
    layerSettings[4] = {
            kLayerName, "syncval_submit_time_validation", kType, kValCount, &syncValSubmitTime};
    layerSettings[5] = {
            kLayerName, "syncval_message_extra_properties", kType, kValCount, &syncValExtraMessage};

    createInfo->sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT;
    createInfo->settingCount = layerSettingCount;
    createInfo->pSettings = layerSettings;
}

void create_debug_messenger(PFN_vkGetInstanceProcAddr getInstProc,
                            VkDebugUtilsMessengerEXT* messenger,
                            const VkInstance& inst) {
    static const VkDebugUtilsMessageSeverityFlagsEXT kSeveritiesToLog =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

    static const VkDebugUtilsMessageTypeFlagsEXT kMessagesToLog =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};
    messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerInfo.messageSeverity = kSeveritiesToLog;
    messengerInfo.messageType = kMessagesToLog;
    messengerInfo.pfnUserCallback = &DebugUtilsMessenger;

    ACQUIRE_VK_INST_PROC_LOCAL_NOCHECK(CreateDebugUtilsMessengerEXT, inst);
    // Register the callback
    grVkCreateDebugUtilsMessengerEXT(inst, &messengerInfo, nullptr, messenger);
}
#endif // SK_ENABLE_VK_LAYERS

bool CreateVkBackendContext(PFN_vkGetInstanceProcAddr getInstProc,
                            skgpu::VulkanBackendContext* ctx,
                            skgpu::VulkanExtensions* extensions,
                            TestVkFeatures* testVkFeatures,
                            VkDebugUtilsMessengerEXT* debugMessenger,
                            uint32_t* presentQueueIndexPtr,
                            const CanPresentFn& canPresent,
                            bool isProtected) {
    if (!getInstProc) {
        return false;
    }
    // We can set the apiVersion to be whatever the highest api we may use in skia. For now we
    // set it to 1.1 since that is the most common Vulkan version on Android devices.
    static const uint32_t kApiVersion = VK_API_VERSION_1_1;
    const uint32_t instanceVersion = get_instance_version(getInstProc, kApiVersion);

    // Initialize VulkanPreferredFeatures
    testVkFeatures->skiaFeatures.init(kApiVersion);

    TArray<VkLayerProperties> instLayers;
    TArray<const char*> instLayerNames;
    TArray<VkExtensionProperties> instExtensions;
    std::vector<const char*> instExtNames;
    if (!gather_test_inst_extensions_and_layers(getInstProc,
                                                remove_patch_version(instanceVersion),
                                                &instLayers,
                                                &instLayerNames,
                                                &instExtensions,
                                                &instExtNames)) {
        return false;
    }

    // Add any additional instance extensions and layers used for debugging to Skia's instance exts
    testVkFeatures->skiaFeatures.addToInstanceExtensions(
            instExtensions.begin(), instExtensions.size(), instExtNames);

    // Set up VkInstanceCreateInfo
    const VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                        /*pNext=*/nullptr,
                                        "vkTestApp",
                                        /*applicationVersion=*/0,
                                        "vkTestEngine",
                                        /*engineVerison=*/0,
                                        kApiVersion };
    VkInstanceCreateInfo instCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                            /*pNext=*/nullptr,
                                            /*flags=*/0,
                                            &appInfo,
                                            (uint32_t)instLayerNames.size(),
                                            instLayerNames.begin(),
                                            (uint32_t)instExtNames.size(),
                                            instExtNames.data() };
    bool hasDebugExt = false;
    *debugMessenger = VK_NULL_HANDLE;

#ifdef SK_ENABLE_VK_LAYERS
    bool hasLayerSettingsExt = false;

    // Determine whether we can use VK_EXT_DEBUG_UTILS and VK_EXT_LAYER_SETTINGS
    for (size_t i = 0; i < instExtNames.size() && !hasDebugExt && !hasLayerSettingsExt; ++i) {
        if (!strcmp(instExtNames[i], VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            hasDebugExt = true;
        } else if (!strcmp(instExtNames[i], VK_EXT_LAYER_SETTINGS_EXTENSION_NAME)) {
            hasLayerSettingsExt = true;
        }
    }

    // Define and populate VkLayerSettings. These must be defined here such that they remain in
    // scope until instance creation.
    VkLayerSettingsCreateInfoEXT layerSettingsCreateInfo = {};
    static const uint8_t kLayerSettingCount = 6;
    VkLayerSettingEXT layerSettings[kLayerSettingCount];
    if (hasDebugExt && hasLayerSettingsExt) {
        populate_layer_settings(&layerSettingsCreateInfo, layerSettings, kLayerSettingCount);
        instCreateInfo.pNext = &layerSettingsCreateInfo;
    }
#endif

    // Create the VkInstance
    VkInstance inst = VK_NULL_HANDLE;
    ACQUIRE_VK_INST_PROC(CreateInstance, VK_NULL_HANDLE);
    VkResult result;
    result = grVkCreateInstance(&instCreateInfo, nullptr, &inst);
    if (result < 0) {
        SkDebugf("vkCreateInstance failed: %d\n", result);
        return false;
    }

    // Define a function for obtaining the appropriate instance / device proc.
    VkDevice device;
    ACQUIRE_VK_INST_PROC(GetDeviceProcAddr, inst);
    auto getProc = [getInstProc, grVkGetDeviceProcAddr](
            const char* proc_name, VkInstance instance, VkDevice device) {
        if (device != VK_NULL_HANDLE) {
            return grVkGetDeviceProcAddr(device, proc_name);
        }
        return getInstProc(instance, proc_name);
    };

#ifdef SK_ENABLE_VK_LAYERS
    // If supported, set up the debug messenger for the VkInstance.
    if (hasDebugExt) {
        create_debug_messenger(getInstProc, debugMessenger, inst);
    }
#endif

    // Obtain the procs we will need for device creation
    ACQUIRE_VK_PROC(EnumeratePhysicalDevices,               inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetPhysicalDeviceProperties,            inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetPhysicalDeviceQueueFamilyProperties, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(CreateDevice,                           inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetDeviceQueue,                         inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(DeviceWaitIdle,                         inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(DestroyDevice,                          inst, VK_NULL_HANDLE);

    // Determine the quantity of physical devices available
    uint32_t gpuCount;
    result = grVkEnumeratePhysicalDevices(inst, &gpuCount, nullptr);
    if (result != VK_SUCCESS) {
        SkDebugf("vkEnumeratePhysicalDevices failed: %d\n", result);
        return destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExt);
    }
    if (!gpuCount) {
        SkDebugf("vkEnumeratePhysicalDevices returned no supported devices.\n");
        return destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExt);
    }
    // Allocate enough storage for all available physical devices. We should be able to just ask for
    // the first one, but a bug in RenderDoc (https://github.com/baldurk/renderdoc/issues/2766)
    // will smash the stack if we do that.
    TArray<VkPhysicalDevice> physDevs(gpuCount);
    result = grVkEnumeratePhysicalDevices(inst, &gpuCount, physDevs.data());
    if (result) {
        SkDebugf("vkEnumeratePhysicalDevices failed: %d\n", result);
        return destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExt);
    }

    // TODO: Find the best match for our needs. For now, just use the first one.
    VkPhysicalDevice physDev = physDevs.front();
    // Get relevant physical device properties
    VkPhysicalDeviceProperties physDeviceProperties;
    grVkGetPhysicalDeviceProperties(physDev, &physDeviceProperties);
    uint32_t physDeviceVersion = std::min(physDeviceProperties.apiVersion, kApiVersion);

    // Query queue family property size, then initialize and populate a list of queue properties.
    uint32_t queueCount;
    grVkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, nullptr);
    if (!queueCount) {
        SkDebugf("vkGetPhysicalDeviceQueueFamilyProperties returned no queues.\n");
        destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExt);
    }
    SkAutoMalloc queuePropsAlloc(queueCount * sizeof(VkQueueFamilyProperties));
    VkQueueFamilyProperties* queueProps = (VkQueueFamilyProperties*)queuePropsAlloc.get();
    grVkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, queueProps);

    // Iterate to find the graphics queue
    uint32_t graphicsQueueIndex = queueCount;
    for (uint32_t i = 0; i < queueCount; i++) {
        if (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueIndex = i;
            break;
        }
    }
    if (graphicsQueueIndex == queueCount) {
        SkDebugf("Could not find any supported graphics queues.\n");
        destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExt);
    }

    // If a present queue is requested (i.e. this function is called with a non-null
    // presentQueueIndexPtr), iterate to find it.
    uint32_t presentQueueIndex = queueCount;
    if (presentQueueIndexPtr) {
        if (!canPresent) {
            SkDebugf("Present queue requested, but canPresent function is empty.\n");
            return destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExt);
        }

        for (uint32_t i = 0; i < queueCount; i++) {
            if (canPresent(inst, physDev, i)) {
                presentQueueIndex = i;
                break;
            }
        }

        if (presentQueueIndex == queueCount) {
            SkDebugf("Could not find any supported present queues.\n");
            return destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExt);
        }
        *presentQueueIndexPtr = presentQueueIndex;
    } else {
        // If no present queue was requested, set use the single graphics queue.
        presentQueueIndex = graphicsQueueIndex;
    }

    TArray<VkLayerProperties> devLayers;
    TArray<const char*> devLayerNames;
    TArray<VkExtensionProperties> devExtensions;
    std::vector<const char*> devExtNames;
    if (!gather_test_device_extensions_and_layers(getProc,
                                                  remove_patch_version(physDeviceVersion),
                                                  inst,
                                                  physDev,
                                                  &devLayers,
                                                  &devLayerNames,
                                                  &devExtensions,
                                                  &devExtNames)) {
        return destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExt);
    }

    // Note: Any struct that setup_features chains must stay in scope until vkCreateDevice. This is
    // why these structs are located in TestVkFeatures and passed in this function.
    if (!setup_features(getProc,
                        inst,
                        physDev,
                        physDeviceVersion,
                        devExtensions,
                        *testVkFeatures,
                        isProtected)) {
        return destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExt);
    }

    // Enable features and extensions that are desired by Skia. This _adds_ to features and
    // extensions already enabled in deviceExtNames and features that aren't done by
    // skiaFeatures itself.
    testVkFeatures->skiaFeatures.addFeaturesToEnable(devExtNames, testVkFeatures->deviceFeatures);
    extensions->init(getProc,
                     inst,
                     physDev,
                     (uint32_t)instExtNames.size(),
                     instExtNames.data(),
                     (uint32_t)devExtNames.size(),
                     devExtNames.data());

    // Create the VkDeviceQueues. We assume no need for swapchain queue - if one is needed, the
    // client will need its own setup code.
    VkDeviceQueueCreateFlags flags = isProtected ? VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT : 0;
    float queuePriorities[1] = { 0.0 };
    const VkDeviceQueueCreateInfo queueInfo[2] = {
            { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
              /*pNext=*/nullptr,
              flags,
              graphicsQueueIndex,
              /*queueCount=*/1,
              queuePriorities },
            { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
              /*pNext=*/nullptr,
              /*flags=*/0,
              presentQueueIndex,
              /*queueCount*/1,
              queuePriorities}};

    uint32_t queueInfoCount = (presentQueueIndex != graphicsQueueIndex) ? 2 : 1;

    const VkDeviceCreateInfo deviceInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                            &testVkFeatures->deviceFeatures,
                                            /*flags=*/0,
                                            queueInfoCount,
                                            queueInfo,
                                            (uint32_t) devLayerNames.size(),
                                            devLayerNames.begin(),
                                            (uint32_t) devExtNames.size(),
                                            devExtNames.data(),
                                            /*ppEnabledFeatures=*/nullptr};
    // Create the VkDevice
    {
#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
        // skbug.com/40040003
        __lsan::ScopedDisabler lsanDisabler;
#endif
        result = grVkCreateDevice(physDev, &deviceInfo, /*pAllocator=*/nullptr, &device);
    }
    if (result) {
        SkDebugf("CreateDevice failed: %d\n", result);
        return destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExt);
    }

    VkQueue queue;
    if (isProtected) {
        ACQUIRE_VK_PROC(GetDeviceQueue2, inst, device);
        SkASSERT(grVkGetDeviceQueue2 != nullptr);
        VkDeviceQueueInfo2 queueInfo2 = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,
                                         /*pNext=*/nullptr,
                                         VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT,
                                         graphicsQueueIndex,
                                         /*queueIndex=*/0 };
        grVkGetDeviceQueue2(device, &queueInfo2, &queue);
    } else {
        grVkGetDeviceQueue(device, graphicsQueueIndex, /*queueIndex=*/0, &queue);
    }

    skgpu::VulkanInterface interface = skgpu::VulkanInterface(
            getProc, inst, device, instanceVersion, physDeviceVersion, extensions);
    SkASSERT(interface.validate(instanceVersion, physDeviceVersion, extensions));

    sk_sp<skgpu::VulkanMemoryAllocator> memoryAllocator = VkTestMemoryAllocator::Make(
            inst, physDev, device, physDeviceVersion, extensions, &interface);

    ctx->fInstance = inst;
    ctx->fPhysicalDevice = physDev;
    ctx->fDevice = device;
    ctx->fQueue = queue;
    ctx->fGraphicsQueueIndex = graphicsQueueIndex;
    ctx->fMaxAPIVersion = kApiVersion;
    ctx->fVkExtensions = extensions;
    ctx->fDeviceFeatures2 = &testVkFeatures->deviceFeatures;
    ctx->fGetProc = getProc;
    ctx->fProtectedContext = skgpu::Protected(isProtected);
    ctx->fMemoryAllocator = memoryAllocator;

    return true;
}

}  // namespace sk_gpu_test

#endif
