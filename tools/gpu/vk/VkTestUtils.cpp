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
                                const uint32_t availableLayerCount,
                                const uint32_t majorSpecVersion,
                                TArray<VkLayerProperties>* layersToUse) {
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
        for (size_t j = 0; j < availableLayerCount; ++j) {
            if (!strcmp(kDebugLayerNames[i], availableLayers[j].layerName) &&
                majorSpecVersion <= remove_patch_version(availableLayers[j].specVersion)) {
                layersToUse->push_back(availableLayers[i]);
            }
        }
    }
}
#endif

static bool init_instance_extensions_and_layers(PFN_vkGetInstanceProcAddr getInstProc,
                                                uint32_t specVersion,
                                                TArray<VkExtensionProperties>* instanceExtensions,
                                                TArray<VkLayerProperties>* instanceLayers) {
    VkResult result;
    SkASSERT(getInstProc);
    ACQUIRE_VK_INST_PROC_LOCAL(EnumerateInstanceExtensionProperties, VK_NULL_HANDLE);
    ACQUIRE_VK_INST_PROC_LOCAL(EnumerateInstanceLayerProperties, VK_NULL_HANDLE);

    // Instance layers
    uint32_t availableLayerCount = 0;
#ifdef SK_ENABLE_VK_LAYERS
    // Determine the quantity of instance layer properties available. Then, create and populate an
    // array of VkLayerProperties.
    result = grVkEnumerateInstanceLayerProperties(&availableLayerCount, /*pProperties=*/nullptr);
    if (result != VK_SUCCESS) {
        return false;
    }
    VkLayerProperties* availableLayers = new VkLayerProperties[availableLayerCount];
    result = grVkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers);
    if (result != VK_SUCCESS) {
        delete[] availableLayers;
        return false;
    }

    // Add any debug-specific layers used in testing to the instance layers
    append_debug_layers(availableLayers,
                        availableLayerCount,
                        remove_patch_version(specVersion),
                        instanceLayers);

    delete[] availableLayers;
#endif // SK_ENABLE_VK_LAYERS

    // Instance extensions via Vulkan implementation and implicitly-enabled layers
    {
        // Determine the quantity of instance extension properties available. Then, create and
        // populate an array of VkExtensionProperties.
        uint32_t extensionCount = 0;
        result = grVkEnumerateInstanceExtensionProperties(
                /*pLayerName=*/nullptr, &extensionCount, /*pProperties=*/nullptr);
        if (result != VK_SUCCESS) {
            return false;
        }
        VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
        result = grVkEnumerateInstanceExtensionProperties(
                /*pLayerName=*/nullptr, &extensionCount, extensions);
        if (result != VK_SUCCESS) {
            delete[] extensions;
            return false;
        }

        // TODO: Look through extensions for names of layers we care about.

        // Add any additional extensions that are used for testing
        for (uint32_t i = 0; i < extensionCount; ++i) {
            if (include_instance_extension(extensions[i].extensionName)) {
                instanceExtensions->push_back() = extensions[i];
            }
        }
        delete [] extensions;
    }

    // Instance extensions via explicitly enabled layers
    availableLayerCount = instanceLayers->size();
    // For each layer, determine the quantity of instance extension properties available. Then,
    // create and populate an array of VkExtensionProperties.
    for (uint32_t layerIndex = 0; layerIndex < availableLayerCount; ++layerIndex) {
        uint32_t extensionCount = 0;
        result = grVkEnumerateInstanceExtensionProperties(
                (*instanceLayers)[layerIndex].layerName, &extensionCount, /*extensions=*/nullptr);
        if (result != VK_SUCCESS) {
            return false;
        }
        VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
        result = grVkEnumerateInstanceExtensionProperties(
                (*instanceLayers)[layerIndex].layerName, &extensionCount, extensions);
        if (result != VK_SUCCESS) {
            delete[] extensions;
            return false;
        }
        for (uint32_t i = 0; i < extensionCount; ++i) {
            if (include_instance_extension(extensions[i].extensionName)) {
                instanceExtensions->push_back() = extensions[i];
            }
        }
        delete[] extensions;
    }

    return true;
}

#define GET_PROC_LOCAL(F, inst, device) PFN_vk ## F F = (PFN_vk ## F) getProc("vk" #F, inst, device)

// bool query_available_device_extensions(VkInstance inst) {
//     GET_PROC_LOCAL(EnumerateDeviceExtensionProperties, inst, VK_NULL_HANDLE);
//     if (!EnumerateDeviceExtensionProperties) {
//         return false;
//     }

// }

#ifdef SK_ENABLE_VK_LAYERS
// Determine available device layers, appending desired debug layers to our list of deviceLayers.
bool append_available_device_debug_layers(const skgpu::VulkanGetProc& getProc,
                                          VkInstance inst,
                                          VkPhysicalDevice physDev,
                                          uint32_t specVersion,
                                          TArray<VkLayerProperties>* deviceLayers) {
    GET_PROC_LOCAL(EnumerateDeviceLayerProperties, inst, /*device=*/VK_NULL_HANDLE);
    if (EnumerateDeviceLayerProperties) {
        return false;
    }

    VkResult result;
    uint32_t availableLayerCount = 0;
    result = EnumerateDeviceLayerProperties(physDev, &availableLayerCount, /*pProperties=*/nullptr);
    if (result != VK_SUCCESS) {
        return false;
    }
    VkLayerProperties* availableLayers = new VkLayerProperties[availableLayerCount];
    result = EnumerateDeviceLayerProperties(physDev, &availableLayerCount, availableLayers);
    if (result != VK_SUCCESS) {
        delete[] availableLayers;
        return false;
    }

    append_debug_layers(
            availableLayers, availableLayerCount, remove_patch_version(specVersion), deviceLayers);

    delete[] availableLayers;
    return true;
}
#endif // SK_ENABLE_VK_LAYERS

// An empty / null layerName means we are checking extensions provided by the Vulkan version or
// implicitly enabled layers. An actual layer name checks device extensions provided by that layer.
static bool append_available_device_extensions(const skgpu::VulkanGetProc& getProc,
                                               VkInstance inst,
                                               VkPhysicalDevice physDev,
                                               const char* layerName,
                                               TArray<VkExtensionProperties>* deviceExtensions) {
    GET_PROC_LOCAL(EnumerateDeviceExtensionProperties, inst, /*device=*/VK_NULL_HANDLE);
    if (EnumerateDeviceExtensionProperties) {
        return false;
    }

    VkResult result;
    uint32_t availableExtensionCount = 0;
    result = EnumerateDeviceExtensionProperties(physDev,
                                                layerName,
                                                &availableExtensionCount,
                                                /*pProperties=*/nullptr);
    if (result != VK_SUCCESS) {
        return false;
    }
    VkExtensionProperties* availableExtensions = new VkExtensionProperties[availableExtensionCount];
    result = EnumerateDeviceExtensionProperties(physDev,
                                                layerName,
                                                &availableExtensionCount,
                                                availableExtensions);
    if (result != VK_SUCCESS) {
        delete[] availableExtensions;
        return false;
    }

    for (uint32_t i = 0; i < availableExtensionCount; ++i) {
        if (include_device_extension(availableExtensions[i].extensionName)) {
            deviceExtensions->push_back() = availableExtensions[i];
        }
    }
    delete[] availableExtensions;

    return true;

}

// Helper to initialize a list of device extensions and layers to use based upon those available
static bool init_device_extensions_and_layers(const skgpu::VulkanGetProc& getProc,
                                              uint32_t specVersion,
                                              VkInstance inst,
                                              VkPhysicalDevice physDev,
                                              TArray<VkExtensionProperties>* deviceExtensions,
                                              TArray<VkLayerProperties>* deviceLayers) {
#ifdef SK_ENABLE_VK_LAYERS
    if (!append_available_device_debug_layers(getProc, inst, physDev, specVersion, deviceLayers)) {
        return false;
    }
#endif


    // Device extensions from Vulkan implementation + implicitly enabled layers:
    if (!append_available_device_extensions(
                getProc, inst, physDev, /*layerName=*/"", deviceExtensions)) {
        return false;
    }

    // Device extensions from explicitly enabled layers:
    uint32_t enabledLayerCount = deviceLayers->size();
    for (uint32_t i = 0; i < enabledLayerCount; ++i) {
        if (!append_available_device_extensions(
                getProc, inst, physDev, (*deviceLayers)[i].layerName, deviceExtensions)) {
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
                destroy_instance(getInstProc, inst, debugMessenger, hasDebugExtension);          \
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
                destroy_instance(getInstProc, inst, debugMessenger, hasDebugExtension); \
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
                             bool hasDebugExtension) {
    if (hasDebugExtension && *debugMessenger != VK_NULL_HANDLE) {
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
                                          bool hasDebugExtension) {
    destroy_instance(getInstProc, inst, debugMessenger, hasDebugExtension);
    return false;
}

static bool setup_features(const skgpu::VulkanGetProc& getProc,
                           VkInstance inst,
                           VkPhysicalDevice physDev,
                           uint32_t physDeviceVersion,
                           const TArray<VkExtensionProperties>& deviceExtensions,
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
            deviceExtensions.begin(), deviceExtensions.size(), testVkFeatures.deviceFeatures);

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
    if (getInstProc == nullptr) {
        return 0;
    }

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

    TArray<VkLayerProperties> instanceLayers;
    TArray<VkExtensionProperties> instanceExtensions;
    if (!init_instance_extensions_and_layers(
                getInstProc, instanceVersion, &instanceExtensions, &instanceLayers)) {
        return false;
    }

    TArray<const char*> instanceLayerNames;
    std::vector<const char*> instanceExtensionNames;
    for (int i = 0; i < instanceLayers.size(); ++i) {
        instanceLayerNames.push_back(instanceLayers[i].layerName);
    }
    for (int i = 0; i < instanceExtensions.size(); ++i) {
        instanceExtensionNames.push_back(instanceExtensions[i].extensionName);
    }

    testVkFeatures->skiaFeatures.init(kApiVersion);
    testVkFeatures->skiaFeatures.addToInstanceExtensions(
            instanceExtensions.begin(), instanceExtensions.size(), instanceExtensionNames);

    const VkApplicationInfo app_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO, // sType
        nullptr,                            // pNext
        "vktest",                           // pApplicationName
        0,                                  // applicationVersion
        "vktest",                           // pEngineName
        0,                                  // engineVerison
        kApiVersion,                        // apiVersion
    };
    VkInstanceCreateInfo instance_create = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,   // sType
            nullptr,                                  // pNext
            0,                                        // flags
            &app_info,                                // pApplicationInfo
            (uint32_t)instanceLayerNames.size(),      // enabledLayerNameCount
            instanceLayerNames.begin(),               // ppEnabledLayerNames
            (uint32_t)instanceExtensionNames.size(),  // enabledExtensionNameCount
            instanceExtensionNames.data(),            // ppEnabledExtensionNames
    };

    bool hasDebugExtension = false;
    *debugMessenger = VK_NULL_HANDLE;

#ifdef SK_ENABLE_VK_LAYERS
    bool hasLayerSettingsExt = false;

    // Determine whether we can use VK_EXT_DEBUG_UTILS and VK_EXT_LAYER_SETTINGS
    for (size_t i = 0; i < instanceExtensionNames.size()
                       && !hasDebugExtension && !hasLayerSettingsExt; ++i) {
        if (!strcmp(instanceExtensionNames[i], VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            hasDebugExtension = true;
        } else if (!strcmp(instanceExtensionNames[i], VK_EXT_LAYER_SETTINGS_EXTENSION_NAME)) {
            hasLayerSettingsExt = true;
        }
    }

    // Define structs which must remain in-scope, populating them if the extensions are available.
    VkLayerSettingsCreateInfoEXT layerSettingsCreateInfo = {};
    static const uint8_t kLayerSettingCount = 6;
    VkLayerSettingEXT layerSettings[kLayerSettingCount];
    if (hasDebugExtension && hasLayerSettingsExt) {
        populate_layer_settings(&layerSettingsCreateInfo, layerSettings, kLayerSettingCount);
        instance_create.pNext = &layerSettingsCreateInfo;
    }
#endif

    STArray<2, VkPhysicalDevice> physDevs;
    VkDevice device;
    VkInstance inst = VK_NULL_HANDLE;

    ACQUIRE_VK_INST_PROC(CreateInstance, VK_NULL_HANDLE);
    VkResult result;
    result = grVkCreateInstance(&instance_create, nullptr, &inst);
    if (result < 0) {
        SkDebugf("vkCreateInstance failed: %d\n", result);
        return false;
    }

    ACQUIRE_VK_INST_PROC(GetDeviceProcAddr, inst);
    auto getProc = [getInstProc, grVkGetDeviceProcAddr](const char* proc_name,
                                                        VkInstance instance, VkDevice device) {
        if (device != VK_NULL_HANDLE) {
            return grVkGetDeviceProcAddr(device, proc_name);
        }
        return getInstProc(instance, proc_name);
    };

#ifdef SK_ENABLE_VK_LAYERS
    if (hasDebugExtension) {
        create_debug_messenger(getInstProc, debugMessenger, inst);
    }
#endif

    ACQUIRE_VK_PROC(EnumeratePhysicalDevices,               inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetPhysicalDeviceProperties,            inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetPhysicalDeviceQueueFamilyProperties, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(CreateDevice,                           inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetDeviceQueue,                         inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(DeviceWaitIdle,                         inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(DestroyDevice,                          inst, VK_NULL_HANDLE);

    uint32_t gpuCount;
    result = grVkEnumeratePhysicalDevices(inst, &gpuCount, nullptr);
    if (result != VK_SUCCESS) {
        SkDebugf("vkEnumeratePhysicalDevices failed: %d\n", result);
        return destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExtension);
    }
    if (!gpuCount) {
        SkDebugf("vkEnumeratePhysicalDevices returned no supported devices.\n");
        return destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExtension);
    }
    // Allocate enough storage for all available physical devices. We should be able to just ask for
    // the first one, but a bug in RenderDoc (https://github.com/baldurk/renderdoc/issues/2766)
    // will smash the stack if we do that.
    physDevs.resize(gpuCount);
    result = grVkEnumeratePhysicalDevices(inst, &gpuCount, physDevs.data());
    if (result) {
        SkDebugf("vkEnumeratePhysicalDevices failed: %d\n", result);
        return destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExtension);
    }
    // We just use the first physical device.
    // TODO: find best match for our needs
    VkPhysicalDevice physDev = physDevs.front();

    VkPhysicalDeviceProperties physDeviceProperties;
    grVkGetPhysicalDeviceProperties(physDev, &physDeviceProperties);
    uint32_t physDeviceVersion = std::min(physDeviceProperties.apiVersion, kApiVersion);

    // query to get the initial queue props size
    uint32_t queueCount;
    grVkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, nullptr);
    if (!queueCount) {
        SkDebugf("vkGetPhysicalDeviceQueueFamilyProperties returned no queues.\n");
        destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExtension);
    }

    SkAutoMalloc queuePropsAlloc(queueCount * sizeof(VkQueueFamilyProperties));
    // now get the actual queue props
    VkQueueFamilyProperties* queueProps = (VkQueueFamilyProperties*)queuePropsAlloc.get();

    grVkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, queueProps);

    // iterate to find the graphics queue
    uint32_t graphicsQueueIndex = queueCount;
    for (uint32_t i = 0; i < queueCount; i++) {
        if (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueIndex = i;
            break;
        }
    }
    if (graphicsQueueIndex == queueCount) {
        SkDebugf("Could not find any supported graphics queues.\n");
        destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExtension);
    }

    // iterate to find the present queue, if needed
    uint32_t presentQueueIndex = queueCount;
    if (presentQueueIndexPtr && canPresent) {
        for (uint32_t i = 0; i < queueCount; i++) {
            if (canPresent(inst, physDev, i)) {
                presentQueueIndex = i;
                break;
            }
        }
        if (presentQueueIndex == queueCount) {
            SkDebugf("Could not find any supported present queues.\n");
            return destroy_instance_return_false(
                    getInstProc, inst, debugMessenger, hasDebugExtension);
        }
        *presentQueueIndexPtr = presentQueueIndex;
    } else {
        // Just setting this so we end up make a single queue for graphics since there was no
        // request for a present queue.
        presentQueueIndex = graphicsQueueIndex;
    }

    TArray<VkLayerProperties> deviceLayers;
    TArray<VkExtensionProperties> deviceExtensions;

    // TODO: Just use VulkanPreferredFeatures here.
    // skgpu::VulkanPreferredFeatures skiaFeatures;
    // skiaFeatures.init(apiVersion);
    // // For Skia testing, we do not need to add additional instance extensions to the app.
    // skiaFeatures.addToInstanceExtensions(...);
    // // Create the instance, choose physical device, query available extensions, decide on
    // // device features to query.
    // skiaFeatures.addFeaturesToQuery(...);
    // // Query features, decide on extensions and features to enable.
    // skiaFeatures.addFeaturesToEnable(...);
    // // create device.

    if (!init_device_extensions_and_layers(getProc, physDeviceVersion,
                                           inst, physDev,
                                           &deviceExtensions,
                                           &deviceLayers)) {
        return destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExtension);
    }

    TArray<const char*> deviceLayerNames;
    std::vector<const char*> deviceExtensionNames;
    for (int i = 0; i < deviceLayers.size(); ++i) {
        deviceLayerNames.push_back(deviceLayers[i].layerName);
    }

    for (int i = 0; i < deviceExtensions.size(); ++i) {
        deviceExtensionNames.push_back(deviceExtensions[i].extensionName);
    }

    // Note: Any struct that setup_features chains must stay in scope until vkCreateDevice. This is
    // why these structs are located in TestVkFeatures and passed in this function.
    if (!setup_features(getProc,
                        inst,
                        physDev,
                        physDeviceVersion,
                        deviceExtensions,
                        *testVkFeatures,
                        isProtected)) {
        return destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExtension);
    }

    // Enable features and extensions that are desired by Skia. This _adds_ to features and
    // extensions already enabled in deviceExtensionNames and features that aren't done by
    // skiaFeatures itself.
    testVkFeatures->skiaFeatures.addFeaturesToEnable(deviceExtensionNames,
                                                     testVkFeatures->deviceFeatures);

    extensions->init(getProc,
                     inst,
                     physDev,
                     (uint32_t)instanceExtensionNames.size(),
                     instanceExtensionNames.data(),
                     (uint32_t)deviceExtensionNames.size(),
                     deviceExtensionNames.data());

    VkDeviceQueueCreateFlags flags = isProtected ? VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT : 0;
    float queuePriorities[1] = { 0.0 };
    // Here we assume no need for swapchain queue
    // If one is needed, the client will need its own setup code
    const VkDeviceQueueCreateInfo queueInfo[2] = {
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
            nullptr,                                    // pNext
            flags,                                      // VkDeviceQueueCreateFlags
            graphicsQueueIndex,                         // queueFamilyIndex
            1,                                          // queueCount
            queuePriorities,                            // pQueuePriorities

        },
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
            nullptr,                                    // pNext
            0,                                          // VkDeviceQueueCreateFlags
            presentQueueIndex,                          // queueFamilyIndex
            1,                                          // queueCount
            queuePriorities,                            // pQueuePriorities
        }
    };
    uint32_t queueInfoCount = (presentQueueIndex != graphicsQueueIndex) ? 2 : 1;

    const VkDeviceCreateInfo deviceInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,        // sType
        &testVkFeatures->deviceFeatures,             // pNext
        0,                                           // VkDeviceCreateFlags
        queueInfoCount,                              // queueCreateInfoCount
        queueInfo,                                   // pQueueCreateInfos
        (uint32_t) deviceLayerNames.size(),          // layerCount
        deviceLayerNames.begin(),                    // ppEnabledLayerNames
        (uint32_t) deviceExtensionNames.size(),      // extensionCount
        deviceExtensionNames.data(),                 // ppEnabledExtensionNames
        nullptr,                                     // ppEnabledFeatures
    };

    {
#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
        // skbug.com/40040003
        __lsan::ScopedDisabler lsanDisabler;
#endif
        result = grVkCreateDevice(physDev, &deviceInfo, /*pAllocator=*/nullptr, &device);
    }
    if (result) {
        SkDebugf("CreateDevice failed: %d\n", result);
        return destroy_instance_return_false(getInstProc, inst, debugMessenger, hasDebugExtension);
    }

    VkQueue queue;
    if (isProtected) {
        ACQUIRE_VK_PROC(GetDeviceQueue2, inst, device);
        SkASSERT(grVkGetDeviceQueue2 != nullptr);
        VkDeviceQueueInfo2 queue_info2 = {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,          // sType
            nullptr,                                        // pNext
            VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT,           // flags
            graphicsQueueIndex,                             // queueFamilyIndex
            0                                               // queueIndex
        };
        grVkGetDeviceQueue2(device, &queue_info2, &queue);
    } else {
        grVkGetDeviceQueue(device, graphicsQueueIndex, 0, &queue);
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
