#pragma once

#include "Vulkan/include/vulkan.h"

#include "ReadFile.h"



static VkDebugUtilsMessengerEXT debugMessenger;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

   
    std::string* path = (std::string*)pUserData;

	writeLine(*path, std::string(pCallbackData->pMessage));

    return VK_FALSE;
}