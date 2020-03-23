#pragma once

#include "vulkan/include/vulkan.h"
#include <vector>
#include <algorithm>

/*
    TODO:
    1) @GraphicDevice::create
        -Allow additional extensions and check if the ones
         required exist for the device
        Note: the create method only searches
        for a graphics queue and a 
        presentation queue and then stops.

        -Have a transfer only queue created.


    *) @struct WorkQueues
        Allow more than just graphics and presentation
        queues.
*/

struct GraphicDeviceQueueReport {
    uint32_t familyIndex;
    uint32_t maxQueueCount;
    
    uint32_t graphicsQueuesToMake;
    bool supportsGraphics;
    bool supportsPresentation;

    uint32_t computeQueuesToMake;
    bool supportsCompute;

    uint32_t transferQueuesToMake;
    bool supportsDedicatedTransfer;


};

/* 
Note:

    The presentation queue and graphics queue
    may be both from the same family.

*/
struct WorkQueues {

    int32_t transferQueueFamilyIndex;
    VkQueue transferQueue;

	int32_t graphicsQueueFamilyIndex;
	VkQueue graphicsQueue;

	int32_t presentationQueueFamilyIndex;
	VkQueue presentationQueue;

};

struct GraphicDevice {

    std::string name;
    bool isCreated;
    int deviceIndex;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;

    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;

    VkPhysicalDeviceType type;
    std::vector<GraphicDeviceQueueReport> queueReports;

    WorkQueues workQueues;

    GraphicDevice(VkPhysicalDevice physicalDevice, int index) {

 
        this->physicalDevice = physicalDevice;
        this->deviceIndex = index;

        vkGetPhysicalDeviceProperties(physicalDevice, &this->properties);
        vkGetPhysicalDeviceFeatures(physicalDevice, &this->features);
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &this->memoryProperties);

        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

        this->queueFamilyProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

        this->isCreated = false;
        this->name = std::string(this->properties.deviceName);
        this->type = this->properties.deviceType;

        std::vector<GraphicDeviceQueueReport> queuesReport;

        for(uint32_t i = 0; i < queueFamilyCount; i++) {
            VkQueueFamilyProperties queueFamily = this->queueFamilyProperties.at(i);

            GraphicDeviceQueueReport currentQueue = {};
            currentQueue.familyIndex = i;
            currentQueue.maxQueueCount = queueFamily.queueCount;

            if(queueFamily.queueFlags == VK_QUEUE_TRANSFER_BIT) {
                currentQueue.supportsDedicatedTransfer = true;
                continue;
            }
        
            if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                currentQueue.supportsGraphics = true;
            }

            if(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                currentQueue.supportsCompute = true;
            }

            this->queueReports.push_back(currentQueue);
        }

        
 
    }

    //This references the report that gets modified outside.
    //     I think in the future one should pass in a report
    //     that is a copy from this ones report

    WorkQueues create(VkInstance instance) {

        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        WorkQueues returnValue = {};
        VkPhysicalDeviceFeatures enabledDeviceFeatures = {};
        std::vector<VkDeviceQueueCreateInfo> queueCreationInfos;
        std::vector<float> queuePriorities;
    
        int graphicsFamilyIndex = -1;
        int presentationFamilyIndex = -1;


        for(GraphicDeviceQueueReport& currentQueueFamily : this->queueReports) {
            if(currentQueueFamily.supportsGraphics) {
                graphicsFamilyIndex = currentQueueFamily.familyIndex;
            }

            if(currentQueueFamily.supportsPresentation) {
                presentationFamilyIndex = currentQueueFamily.familyIndex;
            }

            if(graphicsFamilyIndex > -1 &&
                presentationFamilyIndex > -1) {
                    break; //We have found the necessary queue(s)
                }
        }

        if(graphicsFamilyIndex == presentationFamilyIndex) {
        //Here our presentation and graphics queue come from the
        //  same family.
            queuePriorities.push_back(1.0f); //A priority for each queue created
            VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
            graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            graphicsQueueCreateInfo.pQueuePriorities = queuePriorities.data();
            graphicsQueueCreateInfo.queueFamilyIndex = graphicsFamilyIndex; //is the same as presentation family index
            graphicsQueueCreateInfo.queueCount = 1; //

            queueCreationInfos.push_back(graphicsQueueCreateInfo);

        } else {

            queuePriorities.push_back(1.0f);//A priority for each queue created
            VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
            graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            graphicsQueueCreateInfo.pQueuePriorities = queuePriorities.data();
            graphicsQueueCreateInfo.queueFamilyIndex = graphicsFamilyIndex; 
            graphicsQueueCreateInfo.queueCount = 1;

            queueCreationInfos.push_back(graphicsQueueCreateInfo);

            VkDeviceQueueCreateInfo presentQueueCreateInfo = {};
            presentQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            presentQueueCreateInfo.pQueuePriorities = queuePriorities.data();
            presentQueueCreateInfo.queueFamilyIndex = presentationFamilyIndex; 
            presentQueueCreateInfo.queueCount = 1;

            queueCreationInfos.push_back(presentQueueCreateInfo);

        }


            VkDeviceCreateInfo createDeviceInfo = {};
            createDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createDeviceInfo.pQueueCreateInfos = queueCreationInfos.data();
            createDeviceInfo.queueCreateInfoCount = queueCreationInfos.size();
            createDeviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
            createDeviceInfo.enabledExtensionCount = deviceExtensions.size();
            createDeviceInfo.pEnabledFeatures = &enabledDeviceFeatures;

            VkResult result = vkCreateDevice(this->physicalDevice, &createDeviceInfo, nullptr, &this->logicalDevice);
            returnValue.graphicsQueueFamilyIndex = graphicsFamilyIndex;
            returnValue.presentationQueueFamilyIndex = presentationFamilyIndex;

            vkGetDeviceQueue(this->logicalDevice, graphicsFamilyIndex, 0, &returnValue.graphicsQueue);
            vkGetDeviceQueue(this->logicalDevice, presentationFamilyIndex, 0, &returnValue.presentationQueue);

            if(result == VK_SUCCESS) {
                this->isCreated = true;
                this->workQueues = returnValue;
                return returnValue;
            } else {
                returnValue.graphicsQueueFamilyIndex = -1;
                returnValue.presentationQueueFamilyIndex = -1;

                returnValue.graphicsQueue = VK_NULL_HANDLE;
                returnValue.presentationQueue = VK_NULL_HANDLE;

                return returnValue;
            }
    }

    ~GraphicDevice() {
        if(this->isCreated) {
            vkDeviceWaitIdle(this->logicalDevice);
            vkDestroyDevice(this->logicalDevice, nullptr);
        }
    }
};
