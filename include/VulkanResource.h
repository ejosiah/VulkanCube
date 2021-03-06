#pragma once

#include <vulkan/vulkan.h>

struct VulkanBuffer{

    VulkanBuffer() = default;

    inline VulkanBuffer(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize size)
    : device(device)
    , buffer(buffer)
    , memory(memory)
    , size(size)
    {}

    VulkanBuffer(const VulkanBuffer&) = delete;

    VulkanBuffer(VulkanBuffer&& source) noexcept {
        operator=(static_cast<VulkanBuffer&&>(source));
    }

    VulkanBuffer& operator=(const VulkanBuffer&) = delete;

    VulkanBuffer& operator=(VulkanBuffer&& source) noexcept{
        device = source.device;
        buffer = source.buffer;
        memory = source.memory;

        source.device = VK_NULL_HANDLE;
        source.buffer = VK_NULL_HANDLE;
        source.memory = VK_NULL_HANDLE;

        return *this;
    }

    void copy(void* source, VkDeviceSize size) const {
        void* dest;
        vkMapMemory(device, memory, 0, size, 0, &dest);
        memcpy(dest, source, size);
        vkUnmapMemory(device, memory);
    }

    ~VulkanBuffer(){
        if(buffer){
            vkDestroyBuffer(device, buffer, nullptr);
            vkFreeMemory(device, memory, nullptr);
        }
    }

    operator VkBuffer() const {
        return buffer;
    }

    VkDevice device = VK_NULL_HANDLE;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDeviceSize  size = 0;
};
