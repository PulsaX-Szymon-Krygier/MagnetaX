// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include "VulkanInstance.h"

#if MX_GRAPHICS_VULKAN
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    std::vector<const char*> GetInstanceExts()
    {
        std::vector<const char*> exts;
        exts.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

        // Windows
        #if MX_PLATFORM_WINDOWS
        exts.push_back("VK_KHR_win32_surface");
        #endif

        // Linux & X11
        #if MX_PLATFORM_LINUX && MX_PLATFORM_X11
        exts.push_back("VK_KHR_xlib_surface");
        #endif

        // Apple (Darwin)
        #if MX_PLATFORM_APPLE
        exts.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
        exts.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        #endif

        // Debug mode
        #if MX_GRAPHICS_VULKAN_DEBUG
        exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        #endif

        return exts;
    }

    void DebugLog(const std::string& msg)
    {
        #if MX_GRAPHICS_VULKAN_DEBUG
        std::clog << "MX: " << msg << std::endl;
        #else
        (void)msg;
        #endif
    }

#if MX_GRAPHICS_VULKAN_DEBUG
    bool CheckValidationLayersSupport(const std::vector<const char*>& layers)
    {
        uint32 count = 0;
        vkEnumerateInstanceLayerProperties(&count, nullptr);

        std::vector<VkLayerProperties> avail(count);
        vkEnumerateInstanceLayerProperties(&count, avail.data());

        for (const char* requiredName : layers)
        {
            bool layerFound = false;

            for (const VkLayerProperties& layer : avail)
            {
                if (std::strcmp(requiredName, layer.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) return false;
        }

        return true;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void*)
    {
        if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            std::cerr << callbackData->pMessage << std::endl;
        }

        return VK_FALSE;
    }
#endif
}

bool VulkanInstance::Create()
{
    Destroy();

    DebugLog("Creating Vulkan instance");

    // App info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "MagnetaX Game"; // Add from config later
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "MagnetaX";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = MX_GRAPHICS_VULKAN_MIN_VERSION; // Maybe use highest available?
    appInfo.pNext = nullptr;

    // Instance info
    const std::vector<const char*> instanceExts = GetInstanceExts();

    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = (uint32)instanceExts.size();
    instanceInfo.ppEnabledExtensionNames = instanceExts.data();
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = nullptr;

    // Mandatory flag on Apple/Darwin (because of MoltenVK)
    #if MX_PLATFORM_APPLE
    instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    #endif

    // Validation layers and debug messenger
    #if MX_GRAPHICS_VULKAN_DEBUG
    std::vector<const char*> validationLayers;
    validationLayers.push_back("VK_LAYER_KHRONOS_validation");

    if (CheckValidationLayersSupport(validationLayers))
    {
        instanceInfo.enabledLayerCount = (uint32)validationLayers.size();
        instanceInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        DebugLog("Vulkan validation layer not available");
    }

    VkDebugUtilsMessengerCreateInfoEXT debugMsgInfo{};
    debugMsgInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMsgInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    debugMsgInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT; // Idk maybe check if those types are ok later
    debugMsgInfo.pfnUserCallback = &DebugMessengerCallback;
    debugMsgInfo.pNext = nullptr;

    instanceInfo.pNext = &debugMsgInfo;
    #else
    instanceInfo.pNext = nullptr;
    #endif

    if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS)
    {
        DebugLog("Failed to create Vulkan instance");

        Destroy();
        return false;
    }

    #if MX_GRAPHICS_VULKAN_DEBUG
    if (!CreateDebugMsg(debugMsgInfo))
    {
        DebugLog("Failed to create Vulkan debug messenger");

        Destroy();
        return false;
    }
    #endif

    DebugLog("Vulkan instance created");

    return true;
}

void VulkanInstance::Destroy()
{
    #if MX_GRAPHICS_VULKAN_DEBUG
    DestroyDebugMsg();
    #endif

    if (instance) vkDestroyInstance(instance, nullptr);

    instance = VK_NULL_HANDLE;
}

#if MX_GRAPHICS_VULKAN_DEBUG
bool VulkanInstance::CreateDebugMsg(const VkDebugUtilsMessengerCreateInfoEXT& debugMsgInfo)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (!func) return false;
    if (func(instance, &debugMsgInfo, nullptr, &debugMsg) == VK_SUCCESS) return true;

    return false;
}

void VulkanInstance::DestroyDebugMsg()
{
    if (debugMsg)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

        if (!func) return;

        func(instance, debugMsg, nullptr);
        debugMsg = VK_NULL_HANDLE;
    }
}
#endif
#endif
