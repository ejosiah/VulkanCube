#pragma once

#include "common.h"
#include "VulkanShaderModule.h"

namespace initializers{

    struct ShaderInfo{
        const VulkanShaderModule& module;
        VkShaderStageFlagBits stage;
        const char*  entry = "main";
    };

    static inline std::vector<VkPipelineShaderStageCreateInfo> vertexShaderStages(VkDevice device, const std::vector<ShaderInfo>& shaderInfos){
        std::vector<VkPipelineShaderStageCreateInfo> createInfos;

        for(auto& shaderInfo : shaderInfos){
            VkPipelineShaderStageCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            createInfo.stage = shaderInfo.stage;
            createInfo.module = shaderInfo.module;
            createInfo.pName = shaderInfo.entry;

            createInfos.push_back(createInfo);
        }

        return createInfos;
    }

    static inline VkPipelineVertexInputStateCreateInfo vertexInputState(const std::vector<VkVertexInputBindingDescription>& bindings = {}, const std::vector<VkVertexInputAttributeDescription>& attributes = {}){
        VkPipelineVertexInputStateCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        createInfo.vertexBindingDescriptionCount = COUNT(bindings);
        createInfo.pVertexBindingDescriptions = bindings.data();
        createInfo.vertexAttributeDescriptionCount = COUNT(attributes);
        createInfo.pVertexAttributeDescriptions = attributes.data();

        return createInfo;
    }

    static inline VkPipelineInputAssemblyStateCreateInfo inputAssemblyState(VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VkBool32 primitiveRestart = VK_FALSE){
        VkPipelineInputAssemblyStateCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        createInfo.topology = topology;
        createInfo.primitiveRestartEnable = primitiveRestart;

        return createInfo;
    }

    static inline VkPipelineViewportStateCreateInfo viewportState(const std::vector<VkViewport>& viewports, const std::vector<VkRect2D>& scissors){
        VkPipelineViewportStateCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        createInfo.viewportCount = COUNT(viewports);
        createInfo.pViewports = viewports.data();
        createInfo.scissorCount = COUNT(scissors);
        createInfo.pScissors = scissors.data();

        return createInfo;

    }

    static inline VkPipelineViewportStateCreateInfo viewportState(const VkViewport& viewport, const VkRect2D& scissor){
//        std::vector<VkViewport> viewports{ viewport };
//        std::vector<VkRect2D> scissors{ scissor };
//        return viewportState( viewports, scissors );
        VkPipelineViewportStateCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        createInfo.viewportCount = 1;
        createInfo.pViewports = &viewport;
        createInfo.scissorCount = 1;
        createInfo.pScissors = &scissor;

        return createInfo;
    }

    static inline VkPipelineRasterizationStateCreateInfo rasterizationState(){
        VkPipelineRasterizationStateCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        createInfo.polygonMode = VK_POLYGON_MODE_FILL;
        createInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        createInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        createInfo.lineWidth = 1.0f;

        return createInfo;
    }

    static inline VkPipelineMultisampleStateCreateInfo multisampleState(){
        VkPipelineMultisampleStateCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        createInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        return createInfo;
    }

    static inline VkPipelineDepthStencilStateCreateInfo depthStencilState(){
        VkPipelineDepthStencilStateCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

        return createInfo;
    }

    static inline VkPipelineColorBlendStateCreateInfo colorBlendState(VkPipelineColorBlendAttachmentState blendAttachmentState = {}){
        if(blendAttachmentState.colorWriteMask == 0){
            blendAttachmentState.colorWriteMask =
                      VK_COLOR_COMPONENT_R_BIT
                    | VK_COLOR_COMPONENT_G_BIT
                    | VK_COLOR_COMPONENT_B_BIT
                    | VK_COLOR_COMPONENT_A_BIT;
        }
        VkPipelineColorBlendStateCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = &blendAttachmentState;

        return createInfo;
    }

    static inline VkPipelineDynamicStateCreateInfo dynamicState(std::vector<VkDynamicState> dynamicStates  = {}){
        VkPipelineDynamicStateCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        createInfo.dynamicStateCount = COUNT(dynamicStates);
        createInfo.pDynamicStates = dynamicStates.data();

        return createInfo;
    }

    static inline VkViewport viewport(float width, float height, float x = 0, float y = 0, float minDepth = 0, float maxDepth = 1){
        return { x, y, width, height, minDepth, maxDepth };
    }

    static inline VkRect2D scissor(VkExtent2D extent, VkOffset2D offset = {0, 0}){
        return { offset, extent};
    }
};

