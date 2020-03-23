#include "projectDefs.h"

#include <iostream>

#include <fstream>

#include <thread>
#include <chrono>


#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <streambuf>

#include "VulkanAPI.h"

#include "Display.h"

#include <Windows.h>


#include "GLFW/glfw3.h"
#include "Memory.h"

#include "FillShapeMaterial.h"
#include "Input.h"

#include "LinearAlgebra.h"
#include "Transform.h"

#include "Timeing.h"
#include "Ship.h"


/* ######################################################


*/


struct AppData {
	std::string title;
	uint32_t width;
	uint32_t height;

};

struct DisplaySync {
	VkSemaphore framebufferAvailable;
	VkSemaphore framebufferPresentable;

	VkFence workFinished;
};

	/* NOTES:

		Limiting frame rate is a balance between cpu usage(how long in the wait loop)
		and a consistent frame rate(when the OS decides to wake up the thread).

		Implementing an average of frame rate and useing that to set the waitLoopDuration
		would be a good balance between melting the CPU and having 16 to 20 milliseconds per frame.

		Also, limiting frame rate should preserve power.
	*/


int main() {

#if BUILD_INTERNAL
	initDebug();
#endif

	AppData application;
	application.title = "Light Ship";
	application.width = 1280;
	application.height = 720;
	

	if (!glfwInit()) {
		writeLine(DebugPath, "Failed to init glfw");
	} 

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWwindow* window = glfwCreateWindow(application.width, application.height, application.title.c_str(), NULL, NULL);

	if (!window) {
		writeLine(DebugPath, "Failed to create window");
	}

	Input input;

	glfwSetWindowUserPointer(window, &input);

	glfwSetKeyCallback(window, input.key_callback);
	glfwSetCursorPosCallback(window, input.mouse_pos_callback);
	glfwSetMouseButtonCallback(window, input.mouse_button_callback);


	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	VulkanAPI vulkan;
	std::vector<GraphicDevice> availableGPUs;
	GraphicDevice* selectedDevice;
	uint32_t selectedDeviceIndex;
	WorkQueues workQueues;


	VkSurfaceKHR surface;
	Display display;
	std::vector<VkFramebuffer> framebuffers;

	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

//Todo: swapchain count of these?
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;

	FillShapeMaterial fillShape;

	vulkan.init(extensions);
	VkResult surfaceResult = glfwCreateWindowSurface(vulkan.instance, window, nullptr, &surface);

	if(surfaceResult != VK_SUCCESS) {
		writeLine(DebugPath, "Failed to create a surface");
	}

	availableGPUs = vulkan.getDevices();
	fillPresentationSupport(availableGPUs, surface);

	uint32_t deviceCount = 0;
	for(GraphicDevice& device : availableGPUs) {
		for(GraphicDeviceQueueReport& report : device.queueReports) {
			if(report.supportsGraphics && report.supportsPresentation) {
				selectedDeviceIndex = deviceCount;
				selectedDevice = &device;
				workQueues = selectedDevice->create(vulkan.instance);
				break;
			}
		}
		deviceCount++;
	}


	VkExtent2D hopefullSize = {application.width, application.height};
	uint32_t bufferCount = 2;

	VkResult swapchainResult = display.create(*selectedDevice, surface, bufferCount, hopefullSize);

	if(swapchainResult != VK_SUCCESS) {
		writeLine(DebugPath, "Failed to create swapchain, images or views.");
	}


	VkCommandPoolCreateInfo commandpoolInfo = {};
	commandpoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandpoolInfo.queueFamilyIndex = workQueues.graphicsQueueFamilyIndex;
	commandpoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkResult commandpoolResult = vkCreateCommandPool(selectedDevice->logicalDevice, &commandpoolInfo, nullptr, &commandPool);

	if(commandpoolResult != VK_SUCCESS) {
		writeLine(DebugPath, "Failed to create command pool");
	}

	commandBuffers.resize(display.bufferCount);
	VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.commandPool = commandPool;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

	VkResult cmdBufferAllocResult = vkAllocateCommandBuffers(selectedDevice->logicalDevice, &cmdBufferAllocInfo, commandBuffers.data());
	if(cmdBufferAllocResult != VK_SUCCESS) {
		writeLine(DebugPath, "Failed to allocate command pools.");
	}

	fillShape.create(*selectedDevice, display.format.format);


	framebuffers.resize(display.bufferCount);

	uint32_t framebufferIndex = 0;
	for(VkImageView& view : display.swapchainImageViews) {

		VkImageView attachments[] = {
			view
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = fillShape.renderpass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = display.framebufferSize.width;
		framebufferInfo.height = display.framebufferSize.height;
		framebufferInfo.layers = 1;

		VkResult framebufferResult = vkCreateFramebuffer(
									selectedDevice->logicalDevice, 
									&framebufferInfo, 
									nullptr, 
									&framebuffers[framebufferIndex]);

		if(framebufferResult != VK_SUCCESS) {
			writeLine(DebugPath, "Failed to create framebuffer(s)");
		}
		framebufferIndex++;	
	}





	VkSemaphore framebufferAvailable;
	VkSemaphore framebufferPresentable;

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	

	vkCreateSemaphore(selectedDevice->logicalDevice, &semaphoreInfo, nullptr, &framebufferAvailable);
	vkCreateSemaphore(selectedDevice->logicalDevice, &semaphoreInfo, nullptr, &framebufferPresentable);

	VkFence workCompleted;
	
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	VkResult fenceResult = vkCreateFence(selectedDevice->logicalDevice, &fenceCreateInfo, nullptr, &workCompleted);

	if(fenceResult != VK_SUCCESS) {
		writeLine(DebugPath, "Failed to create frence");
	}

/* Graphics Resource Memory */

	DeviceBuffer attributeBuffer;
	uint32_t memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	uint32_t bufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VkSharingMode queueSharing = VK_SHARING_MODE_EXCLUSIVE;

	attributeBuffer.Create(*selectedDevice, sizeof(float)*64, memoryFlags, bufferUsage, queueSharing);
	//attributeBuffer.UpdateValues(*selectedDevice, 0, sizeof(float)*15, (void*)RainbowTriangle);

	DeviceBuffer uniformBuffer;
	bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	uniformBuffer.Create(*selectedDevice, sizeof(float)*64, memoryFlags, bufferUsage, queueSharing);

/* Descriptor Stuff */

	VkDescriptorPoolSize descPoolSize = {};
	descPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descPoolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo descPoolInfo = {};
	descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolInfo.poolSizeCount = 1;
	descPoolInfo.pPoolSizes = &descPoolSize;
	descPoolInfo.poolSizeCount = 1;
	descPoolInfo.maxSets = 1;

	VkResult descPoolResult = vkCreateDescriptorPool(selectedDevice->logicalDevice, &descPoolInfo, nullptr, &descriptorPool);
	
	if(descPoolResult != VK_SUCCESS) {

	}
	
	VkDescriptorSetAllocateInfo descSetAllocInfo = {};
	descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descSetAllocInfo.descriptorPool = descriptorPool;
	descSetAllocInfo.descriptorSetCount = 1;
	descSetAllocInfo.pSetLayouts = &fillShape.descriptorSetLayout;

	VkResult descSetLayoutResult = vkAllocateDescriptorSets(
		selectedDevice->logicalDevice, &descSetAllocInfo, &descriptorSet);
	
	if(descSetLayoutResult != VK_SUCCESS) {

	}

	VkDescriptorBufferInfo descSetBufferInfo = {};
	descSetBufferInfo.buffer = uniformBuffer.buffer;
	descSetBufferInfo.offset = 0;
	descSetBufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &descSetBufferInfo;
	descriptorWrite.pImageInfo = nullptr; // Optional
	descriptorWrite.pTexelBufferView = nullptr; // Optional

	vkUpdateDescriptorSets(selectedDevice->logicalDevice, 1, &descriptorWrite, 0, nullptr);
/* End uniform buffer */


	float clearColor[4] = {.9f, 0.9f, 0.9f, 1.0f};
	float deltaColor = .01f;


	uint32_t currentBuffer = 0;


	double start_t = 0.0;
	double end_t = 0.0;

	GameTime time = {0};

	FrameRate frameRate = {0};
	frameRate.limitFrameRate = true;
	frameRate.fps = 62.0f;
	frameRate.targetFrameDuration = 1.0/frameRate.fps;
	frameRate.waitLoopDuration = 15;

	glfwSetTime(0);


	View camera;
	camera.resolution = {(float)application.width, (float)application.height};
	camera.unitRatio = 64.0f;


	MemoryRange memRange;
	memRange.offset = 0;
	memRange.length = sizeof(float)*30;

	Ship testShip(fillShape, memRange, TEST_SHIP_DATA);
	testShip.transform.position = {0.0f, 0.0f};
	testShip.transform.scale = .75f;

	attributeBuffer.UpdateValues(*selectedDevice, memRange, testShip.attributes);

	MemoryRange viewUpdateRange;
	viewUpdateRange.offset = 0;
	viewUpdateRange.length = View::SIZE;

	float offset = 0.0;
	while (!glfwWindowShouldClose(window)) {
		start_t = glfwGetTime();
		frameRate.frameDuration = start_t;
		
		input.update();
		input.Mouse.worldPosition = camera.screenToWorld(input.Mouse.screenPosition);
		
/* Update */		

		
		testShip.Update(input, time);

/*End Update*/


		uniformBuffer.UpdateValues(*selectedDevice, viewUpdateRange, camera.pack());

		VkRect2D scissor = {};
		scissor.offset = {0, 0};
		scissor.extent = display.framebufferSize;

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = (float)display.framebufferSize.width;
		viewport.height = (float)display.framebufferSize.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

/* Render */
		uint32_t loop_frameBufferIndex;
		vkAcquireNextImageKHR(selectedDevice->logicalDevice, 
							display.swapchain, 
							UINT64_MAX, 
							framebufferAvailable, 
							VK_NULL_HANDLE,
							&loop_frameBufferIndex);


/*Command Buffer Recording*/

 	VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(commandBuffers[loop_frameBufferIndex], &beginInfo);
		VkClearValue clearValue = {clearColor[0], clearColor[1], clearColor[2], clearColor[3]};
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = fillShape.renderpass;
		renderPassInfo.framebuffer = framebuffers[loop_frameBufferIndex];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = display.framebufferSize;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(commandBuffers[loop_frameBufferIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdSetScissor(commandBuffers[loop_frameBufferIndex], 0, 1, &scissor);
		vkCmdSetViewport(commandBuffers[loop_frameBufferIndex], 0, 1, &viewport);


		vkCmdBindPipeline(commandBuffers[loop_frameBufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, fillShape.pipeline);

		vkCmdBindDescriptorSets(commandBuffers[loop_frameBufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, fillShape.layout, 0, 1, &descriptorSet, 0, nullptr);

		VkBuffer referenceBuffers[] = {attributeBuffer.buffer};
		VkDeviceSize memoryOffsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffers[loop_frameBufferIndex], 0, 1, referenceBuffers, memoryOffsets);

			testShip.Draw(commandBuffers[loop_frameBufferIndex]);

	    vkCmdEndRenderPass(commandBuffers[loop_frameBufferIndex]);

	vkEndCommandBuffer(commandBuffers[loop_frameBufferIndex]);

/*End Command Buffer Recording*/




		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitOn[] = {framebufferAvailable};
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitOn;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[loop_frameBufferIndex];

		VkSemaphore signalSemaphores[] = {framebufferPresentable};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;


		vkQueueSubmit(workQueues.graphicsQueue, 1, &submitInfo, workCompleted);
	
						
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &framebufferPresentable;
		
		VkSwapchainKHR swapChains[] = {display.swapchain};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &loop_frameBufferIndex;

		vkQueuePresentKHR(workQueues.presentationQueue, &presentInfo);

/* End Render */
		vkWaitForFences(selectedDevice->logicalDevice, 1, &workCompleted, VK_TRUE, UINT64_MAX);
		vkResetFences(selectedDevice->logicalDevice, 1, &workCompleted);


		if(frameRate.limitFrameRate) {

			frameRate.frameDuration = glfwGetTime() - start_t;
			double toMilli = 1000;
			long long timeRemaining = static_cast<long long>(toMilli*(frameRate.targetFrameDuration - frameRate.frameDuration)) - frameRate.waitLoopDuration;
		
			std::this_thread::sleep_for(std::chrono::milliseconds(timeRemaining));

			frameRate.frameDuration = glfwGetTime() - start_t;
			while(frameRate.frameDuration < frameRate.targetFrameDuration) {
				frameRate.frameDuration = glfwGetTime() - start_t;
			}
		}
	
		//std::cout << time.dt << std::endl;

		currentBuffer++;

		end_t = glfwGetTime();
		time.dt = end_t - start_t;
		time.totalTime = end_t;

	}



	vkDestroyFence(selectedDevice->logicalDevice, workCompleted, nullptr);
	vkDestroySemaphore(selectedDevice->logicalDevice, framebufferAvailable, nullptr);
	vkDestroySemaphore(selectedDevice->logicalDevice, framebufferPresentable, nullptr);

	
	attributeBuffer.Destroy(*selectedDevice);
	uniformBuffer.Destroy(*selectedDevice);

	for(VkFramebuffer& framebuffer : framebuffers) {
		vkDestroyFramebuffer(selectedDevice->logicalDevice, framebuffer, nullptr);
	}


	fillShape.destroy(*selectedDevice);
	vkDestroyDescriptorPool(selectedDevice->logicalDevice, descriptorPool, nullptr);
	vkDestroyCommandPool(selectedDevice->logicalDevice, commandPool, nullptr);
	//the display must be destroyed before the surface
	display.Destroy(availableGPUs[selectedDeviceIndex]);
	vkDestroySurfaceKHR(vulkan.instance, surface, nullptr);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}