#pragma once

#include "common.h"

struct VulkanDescriptorSet{
    DISABLE_COPY(VulkanDescriptorSet)

    VulkanDescriptorSet() = default;

    VulkanDescriptorSet(VkDevice device, VkDescriptorPool pool, VkDescriptorSet descriptorSet)
    :device(device),
    pool(pool),
    descriptorSet(descriptorSet)
    {

    }

    VulkanDescriptorSet(VulkanDescriptorSet&& source) noexcept {
        operator=(static_cast<VulkanDescriptorSet&&>(source));
    }

    VulkanDescriptorSet& operator=(VulkanDescriptorSet&& source) noexcept {
        if(this == &source) return *this;
        this->device = source.device;
        this->pool = source.pool;
        this->descriptorSet = source.descriptorSet;

        source.descriptorSet = VK_NULL_HANDLE;

        return *this;
    }

    ~VulkanDescriptorSet(){
        if(descriptorSet){
            vkFreeDescriptorSets(device, pool, 1, &descriptorSet);
        }
    }

    operator VkDescriptorSet() const{
        return descriptorSet;
    }

    VkDevice device = VK_NULL_HANDLE;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
};

struct VulkanDescriptorPool{

    DISABLE_COPY(VulkanDescriptorPool)

    VulkanDescriptorPool() = default;

    VulkanDescriptorPool(VkDevice device, uint32_t maxSet, const std::vector<VkDescriptorPoolSize>& poolSizes)
    :device(device)
    {
        VkDescriptorPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.maxSets = maxSet;
        createInfo.poolSizeCount = COUNT(poolSizes);
        createInfo.pPoolSizes = poolSizes.data();

        ASSERT(vkCreateDescriptorPool(device, &createInfo, nullptr, &pool));
    }

    VulkanDescriptorPool(VulkanDescriptorPool&& source) noexcept {
        operator=(static_cast<VulkanDescriptorPool&&>(source));
    }

    ~VulkanDescriptorPool(){
        if(pool){
            vkDestroyDescriptorPool(device, pool, VK_NULL_HANDLE);
        }
    }

    VulkanDescriptorPool& operator=(VulkanDescriptorPool&& source) noexcept {
        if(this == &source) return *this;

        this->device = source.device;
        this->pool = source.pool;

        source.pool = VK_NULL_HANDLE;

        return *this;
    }

    [[nodiscard]]
    std::vector<VulkanDescriptorSet> allocate(const std::vector<VkDescriptorSetLayout>& layouts) const {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = COUNT(layouts);
        allocInfo.pSetLayouts = layouts.data();

        std::vector<VkDescriptorSet> sets(layouts.size());
        vkAllocateDescriptorSets(device, &allocInfo, sets.data());

        std::vector<VulkanDescriptorSet> vSets;
        for(auto& set : sets){
            vSets.emplace_back(device, pool, set);
        }
        return vSets;
    }

    operator VkDescriptorPool() const {
        return pool;
    }

    VkDevice device = VK_NULL_HANDLE;
    VkDescriptorPool pool = VK_NULL_HANDLE;
};

