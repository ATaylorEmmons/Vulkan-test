#pragma once

#include "vulkan/include/vulkan.h"

#if BUILD_INTERNAL
#include "vulkanAPI_debug.h"
#endif

#include "GraphicDevice.h"

#include <vector>


struct VulkanAPI {

    VkInstance instance;

    std::vector<GraphicDevice> devices;



    /* 
    TODO:
    _________________
    -Use the instance pNext to get better debug info.
        o Right now the surfaceKHR object not being destroyed
          doesn't get caught.(Except it gets printed to the console?)

    */
    VulkanAPI() { }

    void init(std::vector<const char*>& extensions) {

        VkInstanceCreateInfo instanceInfo = {};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pApplicationInfo = NULL;

    #if BUILD_INTERNAL  
        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instanceInfo.enabledLayerCount = validationLayers.size();
        instanceInfo.ppEnabledLayerNames = validationLayers.data();
    #else
        instanceInfo.enabledLayerCount = 0;
    #endif

        instanceInfo.enabledExtensionCount = extensions.size();
        instanceInfo.ppEnabledExtensionNames = extensions.data();


        VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);
        
        
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;
        debugCreateInfo.pUserData = &DebugPath;


        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, &debugCreateInfo, nullptr, &debugMessenger);
        } else {
            writeLine(DebugPath, "Debug Messenger Extension not available.");
        }

        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

        if(deviceCount <= 0) {
            writeLine(DebugPath, "No vulkan compatible GPUs on system.");
        }

        for(uint32_t i = 0; i < deviceCount; i++) {
            GraphicDevice graphicDevice(physicalDevices.at(i), i);
            this->devices.push_back(graphicDevice);
        }

    }

    std::vector<GraphicDevice> getDevices() {
        return this->devices;
    }



    ~VulkanAPI() {
        
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, nullptr);
        } else {
             writeLine(DebugPath, "Debug Messenger Extension not available.");
        }

        vkDestroyInstance(instance, nullptr);
    }


};