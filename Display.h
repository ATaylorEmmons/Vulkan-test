
#include <algorithm>
#include <vector>
#include <string>

#include "VulkanAPI.h"
#include "ReadFile.h"

static void fillPresentationSupport(std::vector<GraphicDevice>& devices, VkSurfaceKHR surface) {
	for(GraphicDevice& currentDevice : devices) {
		VkBool32 presentationSupport = false;

		for(GraphicDeviceQueueReport& currentQueueFamily : currentDevice.queueReports) {
			vkGetPhysicalDeviceSurfaceSupportKHR(
				currentDevice.physicalDevice,
				currentQueueFamily.familyIndex,
				surface,
				&presentationSupport);

			currentQueueFamily.supportsPresentation = (presentationSupport == VK_TRUE);
		}
	}
}


/*
    TODO: 
        -Move framebuffers to here
        -Rename framebufferSize to extent
        -Implement swapchain recreation
        -Implement monitor surface rendering
*/

struct Display {

	VkSwapchainKHR swapchain;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;

	VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

	VkExtent2D framebufferSize;
	VkSurfaceFormatKHR format;
	VkPresentModeKHR presentMode;
    uint32_t bufferCount;

    Display( ){}

    //* Caution: don't call until the device has been created
   VkResult create( GraphicDevice& graphicDevice,
                    VkSurfaceKHR surface,
                    uint32_t imageCount, 
                    VkExtent2D extent) {

        this->bufferCount = imageCount;

        getImageSize(graphicDevice, surface, extent);
        getImageCount(graphicDevice, bufferCount);
        getFormats(graphicDevice, surface);
        getPresentationModes(graphicDevice, surface);


        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = bufferCount;
        createInfo.imageFormat = format.format;
        createInfo.imageColorSpace = format.colorSpace;
        createInfo.imageExtent = framebufferSize;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        uint32_t queueIndices[2] = {
		    (uint32_t)graphicDevice.workQueues.graphicsQueueFamilyIndex,
		    (uint32_t)graphicDevice.workQueues.presentationQueueFamilyIndex
        };

        if(queueIndices[0] != queueIndices[1]) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        VkResult result = vkCreateSwapchainKHR(graphicDevice.logicalDevice, &createInfo, nullptr, &swapchain);

        if(result == VK_SUCCESS) {
            uint32_t createdImageCount;
            vkGetSwapchainImagesKHR(graphicDevice.logicalDevice, swapchain, &createdImageCount, nullptr);

            if(createdImageCount != bufferCount) {
                writeLine(DebugPath, std::to_string(createdImageCount) + " number of images. Expected " + std::to_string(bufferCount));
                return result;
            }
            swapchainImages.resize(createdImageCount);
            result = vkGetSwapchainImagesKHR(graphicDevice.logicalDevice, swapchain, &createdImageCount, swapchainImages.data());
              if(result != VK_SUCCESS) {
                  return result;
              }
        } else {
            return result;
        }

        this->swapchainImageViews.resize(bufferCount);
        for(uint32_t currentImageIndex = 0; currentImageIndex < bufferCount; currentImageIndex++) {
         
            VkImageViewCreateInfo viewCreateInfo = {};
            viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewCreateInfo.image = swapchainImages[currentImageIndex];
            viewCreateInfo.format = format.format;
            viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

            viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewCreateInfo.subresourceRange.baseMipLevel = 0;
            viewCreateInfo.subresourceRange.levelCount = 1;
            viewCreateInfo.subresourceRange.baseArrayLayer = 0;
            viewCreateInfo.subresourceRange.layerCount = 1;

            vkCreateImageView(graphicDevice.logicalDevice, &viewCreateInfo, nullptr, &this->swapchainImageViews[currentImageIndex]);
       
            
        }

        return result;

    }

    void Destroy(GraphicDevice& graphicDevice) {
        for(VkImageView& view : this->swapchainImageViews) {
           vkDestroyImageView(graphicDevice.logicalDevice,  view, nullptr);
        }
        vkDestroySwapchainKHR(graphicDevice.logicalDevice, this->swapchain, nullptr);
    }

    ~Display() {
       
    }

private:
    void getImageSize(GraphicDevice& graphicDevice, VkSurfaceKHR surface, VkExtent2D extent) {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphicDevice.physicalDevice, surface, &capabilities);
        if (capabilities.currentExtent.width != UINT32_MAX) {
            framebufferSize = capabilities.currentExtent;
        } else {
            framebufferSize.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
            framebufferSize.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));
        }
    }

    void getImageCount(GraphicDevice& graphicDevice, uint32_t imageCount) {
        if(capabilities.maxImageCount == 0) { 
		//The device doesn't enforce a limit on the number of images
            bufferCount = imageCount;
        }
        else if(bufferCount <= capabilities.minImageCount) {
            bufferCount = capabilities.minImageCount;
                
        } else if(bufferCount >= capabilities.maxImageCount) {
            bufferCount = capabilities.maxImageCount;
        }
    }

    void getFormats(GraphicDevice& graphicDevice, VkSurfaceKHR surface) {
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(graphicDevice.physicalDevice, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(graphicDevice.physicalDevice, surface, &formatCount, formats.data());
        }

        for (const auto& availableFormat : formats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                format = availableFormat;
                break;
            }

                format = formats[0];
        }
    }

    void getPresentationModes(GraphicDevice& graphicDevice, VkSurfaceKHR surface) {
        uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(graphicDevice.physicalDevice, surface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(graphicDevice.physicalDevice, surface, &presentModeCount, presentModes.data());
            }

            for (const auto& availablePresentMode : presentModes) {
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    presentMode = availablePresentMode;
                    break;
                }

                presentMode = VK_PRESENT_MODE_FIFO_KHR;
            }
    }

};