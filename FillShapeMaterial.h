#pragma once


#include "GraphicDevice.h"
#include "ReadFile.h"

#include "VertexDescription.h"

/*
   TODO: 
        -Memory leaks can happen if some stuff gets 
            made but later other stuff fails to get made.

        -Group pipelines and vkRenderPasses into render phases
           -Each RenderPhase wraps up one RenderPass invocation

*/

struct FillShapeMaterial {
    std::string vertexPath = "assets/pipelines/fillShape_vert.svp";
    std::string fragmentPath = "assets/pipelines/fillShape_frag.svp";


    uint32_t subpassIndex = 0;

    VkRenderPass renderpass;

    VkPipeline pipeline;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout layout;

    VkShaderModule vertexModule;
    VkShaderModule fragmentModule;
    
    bool isCreated = false;

    VkResult create(GraphicDevice& device, VkFormat presentationFormat) {

/*_--- Renderpass Creation */

 /* Attachment List */
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = presentationFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

/* Attachment References */
        VkAttachmentReference colorReference = {};
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

/* Subpass List */
        VkSubpassDescription colorOutput = {};
        colorOutput.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        //Attachments this subpass would read from in the fragment shader
        colorOutput.inputAttachmentCount = 0;
        colorOutput.pInputAttachments = nullptr;

        colorOutput.colorAttachmentCount = 1;
        colorOutput.pColorAttachments = &colorReference;
        colorOutput.pResolveAttachments = 0; //For multisample
        colorOutput.pDepthStencilAttachment = nullptr;
        colorOutput.preserveAttachmentCount = 0;
        colorOutput.pPreserveAttachments = nullptr;

/* Dependencies */
        VkSubpassDependency fromPresentation = {};
        fromPresentation.srcSubpass = VK_SUBPASS_EXTERNAL;
        fromPresentation.dstSubpass = 0;

        fromPresentation.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        fromPresentation.srcAccessMask = 0;

        fromPresentation.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        //Only need read bit?
        fromPresentation.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        
        VkRenderPassCreateInfo renderpassInfo = {};
        renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpassInfo.attachmentCount = 1;
        renderpassInfo.pAttachments = &colorAttachment;
        renderpassInfo.subpassCount = 1;
        renderpassInfo.pSubpasses = &colorOutput;
        renderpassInfo.dependencyCount = 1;
        renderpassInfo.pDependencies = &fromPresentation;
        
        VkResult renderpassResult = vkCreateRenderPass(
                device.logicalDevice,
                &renderpassInfo, 
                nullptr, 
                &renderpass);
 
        if(renderpassResult != VK_SUCCESS) {
                
        }

/*_--- Pipeline Creation */  


/* Shaders */

        std::vector<char> vertexByteCode = readBinary(vertexPath);
        std::vector<char> fragmentByteCode = readBinary(fragmentPath);

        VkShaderModuleCreateInfo vertexShaderModuleInfo = {};
        vertexShaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        
        vertexShaderModuleInfo.codeSize = vertexByteCode.size();
        vertexShaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(vertexByteCode.data());

        VkShaderModuleCreateInfo fragmentShaderModuleInfo = {};
        fragmentShaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        fragmentShaderModuleInfo.codeSize = fragmentByteCode.size();
        fragmentShaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentByteCode.data());

        vkCreateShaderModule(device.logicalDevice,  &vertexShaderModuleInfo, nullptr, &vertexModule);
        vkCreateShaderModule(device.logicalDevice,  &fragmentShaderModuleInfo, nullptr, &fragmentModule);

        VkPipelineShaderStageCreateInfo vertexStage = {};
        vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexStage.pName = "main";
        vertexStage.module = vertexModule;

        VkPipelineShaderStageCreateInfo fragmentStage = {};
        fragmentStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentStage.pName = "main";
        fragmentStage.module = fragmentModule;

        VkPipelineShaderStageCreateInfo shaderStages[2] = {vertexStage, fragmentStage};

   
/* Vertex Specifications */
        //This dictates which buffer(s) the data is sourced from
        VkVertexInputBindingDescription attributeBindingInfo = {};
        attributeBindingInfo.binding = 0;

        //vec2 + vec3 {+padding?};
        attributeBindingInfo.stride = sizeof(VertexDescriptions::POS2_COLOR3);
        attributeBindingInfo.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

//TODO: Wrap these up with the VERTEXDESCRIPTION
        //These dictate how the data gets broken up into the relevent pieces
        //vec2
        VkVertexInputAttributeDescription positionDescription = {};
        positionDescription.binding = 0;
        positionDescription.location = 0;
        positionDescription.format = VK_FORMAT_R32G32_SFLOAT;
        positionDescription.offset = offsetof(VertexDescriptions::POS2_COLOR3, position);

        //vec3
        VkVertexInputAttributeDescription colorDescription = {};
        colorDescription.binding = 0;
        colorDescription.location = 1;
        colorDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
        colorDescription.offset = offsetof(VertexDescriptions::POS2_COLOR3, color);


        VkVertexInputBindingDescription bindingDescriptionArr[1] = {attributeBindingInfo};
        VkVertexInputAttributeDescription attributeDescriptionArr[2] = { positionDescription, colorDescription};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptionArr;
        vertexInputInfo.vertexAttributeDescriptionCount = 2;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptionArr;


/* Input Assembly */
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
        inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

/* Viewport and Scissor */
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = 0.0f;
        viewport.height = 0.0f;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = {0, 0};

        VkPipelineViewportStateCreateInfo viewportScissorState = {};
        viewportScissorState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportScissorState.viewportCount = 1;
        viewportScissorState.pViewports = &viewport;
        viewportScissorState.scissorCount = 1;
        viewportScissorState.pScissors = &scissor;

 /* Rasterization */

        VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
        rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationInfo.depthClampEnable = VK_FALSE;
        rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationInfo.lineWidth = 1.0f;
        rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationInfo.depthBiasEnable = VK_FALSE;
        rasterizationInfo.depthBiasConstantFactor = 0.0f;
        rasterizationInfo.depthBiasClamp = 0.0f;
        rasterizationInfo.depthBiasSlopeFactor = 0.0f;

/* Multisampling */

        VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
        multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleInfo.sampleShadingEnable = VK_FALSE;
        multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleInfo.minSampleShading = 1.0f; // Optional
        multisampleInfo.pSampleMask = nullptr; // Optional
        multisampleInfo.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampleInfo.alphaToOneEnable = VK_FALSE; // Optional

/* Depth and Stencil  

        VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};
        depthStencilInfo.sType = ;
        depthStencilInfo.depthTestEnable = ;
        depthStencilInfo.depthBoundsTestEnable = ; 
        depthStencilInfo.depthWriteEnable = ;
        depthStencilInfo.depthCompareOp = ;
        depthStencilInfo.minDepthBounds = ;
        depthStencilInfo.maxDepthBounds = ;

        depthStencilInfo.stencilTestEnable = ;
        depthStencilInfo.back = ;
        depthStencilInfo.front = ;
 
 */       

/*Blending */

        //These are per attachment
        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

/* Dynamic States */

        VkDynamicState dynamicStates[2] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

/*_--- Layout Creation */


        VkDescriptorSetLayoutBinding viewBinding = {};
        viewBinding.binding = 0;
        viewBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        viewBinding.descriptorCount = 1;
        viewBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        viewBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutSetInfo = {};
        layoutSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutSetInfo.bindingCount = 1;
        layoutSetInfo.pBindings = &viewBinding;

        VkResult descSetLayout = vkCreateDescriptorSetLayout(
                                                        device.logicalDevice,
                                                        &layoutSetInfo, 
                                                        nullptr, 
                                                        &descriptorSetLayout);
   
        if(descSetLayout != VK_SUCCESS) {
                
        }


/* Push Constant Range  MIN: 128 bytes: 32 floats */

        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = MAX_PUSHCONSTANT_SIZE;


        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1; 
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; 

        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;



        VkResult layoutResult = vkCreatePipelineLayout(device.logicalDevice, &pipelineLayoutInfo, nullptr, &layout);

        if(layoutResult != VK_SUCCESS) {
                
        }


/*_--- Pipline Creation */

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
        pipelineInfo.pViewportState = &viewportScissorState;
        pipelineInfo.pRasterizationState = &rasterizationInfo;
        pipelineInfo.pMultisampleState = &multisampleInfo;
        pipelineInfo.pDepthStencilState = nullptr; 
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState; 
        pipelineInfo.layout = layout;
        pipelineInfo.renderPass = renderpass;
        pipelineInfo.subpass = subpassIndex;

        VkResult pipelineResult = vkCreateGraphicsPipelines(
                                        device.logicalDevice,
                                        VK_NULL_HANDLE,
                                        1,
                                        &pipelineInfo,
                                        nullptr,
                                        &pipeline);

        if(pipelineResult != VK_SUCCESS) {

        }

        this->isCreated = true;
        return VK_SUCCESS;
    }

    void destroy(GraphicDevice& device) {
        if(this->isCreated) {
            vkDestroyShaderModule(device.logicalDevice, vertexModule, nullptr);
            vkDestroyShaderModule(device.logicalDevice, fragmentModule, nullptr);

            vkDestroyRenderPass(device.logicalDevice, renderpass, nullptr);
            vkDestroyDescriptorSetLayout(device.logicalDevice, descriptorSetLayout, nullptr);
            vkDestroyPipelineLayout(device.logicalDevice, layout, nullptr);
            vkDestroyPipeline(device.logicalDevice, pipeline, nullptr);
        }
    }

};