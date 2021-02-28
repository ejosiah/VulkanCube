#pragma once

#include "common.h"

struct VulkanPipeline{

    VulkanPipeline() = default;

    VulkanPipeline(VkDevice device, VkPipeline pipeline )
    : device(device)
    , pipeline(pipeline)
    {
    }

    VulkanPipeline(const VulkanPipeline& ) = delete;

    VulkanPipeline(VulkanPipeline&& source) noexcept {
        operator=(static_cast<VulkanPipeline&&>(source));
    }

    ~VulkanPipeline(){
        if(pipeline){
            vkDestroyPipeline(device, pipeline, nullptr);
        }
    }

    VulkanPipeline& operator=(const VulkanPipeline&) = delete;

    VulkanPipeline& operator=(VulkanPipeline&& source) noexcept {
        if(this == &source) return *this;
        this->device = source.device;
        this->pipeline = source.pipeline;

        source.pipeline = VK_NULL_HANDLE;
        return *this;
    }

    operator VkPipeline() const {
        return pipeline;
    }

    VkDevice device = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
};
