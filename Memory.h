#pragma once

#include "GraphicDevice.h"


struct MemoryRange {
    uint32_t offset;
    uint32_t length;
};

struct DeviceBuffer {

    VkBuffer buffer;
    VkDeviceMemory memory;
    uint32_t sizeInBytes;
    uint32_t bytesRemaining;

    DeviceBuffer() {}

    void Create(GraphicDevice& device, 
                 uint32_t size, 
                 uint32_t memoryTypeFlags,
                 uint32_t bufferUsageFlags, 
                 VkSharingMode queueFamilyUsage) {

        this->sizeInBytes = size;
        this->bytesRemaining = this->sizeInBytes;

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeInBytes;
        bufferInfo.usage = bufferUsageFlags;
        bufferInfo.sharingMode = queueFamilyUsage;

        VkResult createAttrBufferResult = vkCreateBuffer(
										device.logicalDevice,
										&bufferInfo,
										nullptr,
										&buffer);
        
        VkMemoryRequirements attrMemRequs;
        vkGetBufferMemoryRequirements(
                device.logicalDevice, 
                buffer,
                &attrMemRequs);

        uint32_t memoryIndex;
        for (uint32_t i = 0; i < device.memoryProperties.memoryTypeCount; i++) {
            if ((attrMemRequs.memoryTypeBits & (1 << i)) && (device.memoryProperties.memoryTypes[i].propertyFlags & memoryTypeFlags) == memoryTypeFlags) {
                memoryIndex = i;
                break;
            }
        }

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = attrMemRequs.size;
        allocInfo.memoryTypeIndex = memoryIndex;

        VkResult allocationResult = vkAllocateMemory(device.logicalDevice, &allocInfo, nullptr, &memory);
        vkBindBufferMemory(device.logicalDevice, buffer, memory, 0);

        if(allocationResult != VK_SUCCESS) {
            if(allocationResult == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
                //TODO: Memory plan b(spill over to main memory)
            }
        }
    }
/* TODO: 

    Error check for memory out of bounds and stuff

*/
    void UpdateValues(GraphicDevice& device, MemoryRange memRange, void* data) {
        if((memRange.offset + memRange.length) < this->sizeInBytes) {
            void* memoryLocation;
            vkMapMemory(device.logicalDevice, memory, memRange.offset, memRange.length, 0, &memoryLocation);
            memcpy(memoryLocation, data, (size_t) memRange.length);
            vkUnmapMemory(device.logicalDevice, memory);	
            
        } else {
            //Out of memory
        }
    }

    void Destroy(GraphicDevice& device) {

        vkDestroyBuffer(device.logicalDevice, buffer, nullptr);
        vkFreeMemory(device.logicalDevice, memory, nullptr);

    }
};
