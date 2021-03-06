#pragma once

#include "common.h"
#include "io.h"

struct VulkanShaderModule{

    DISABLE_COPY(VulkanShaderModule)

    VulkanShaderModule() = default;

    explicit VulkanShaderModule(VkDevice device, const io::fs::path& path)
    :device(device)
    {
        auto code = io::load(path);
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = COUNT(code);
        createInfo.pCode = reinterpret_cast<uint32_t*>(code.data());

        ASSERT(vkCreateShaderModule(device, &createInfo, nullptr, &module));
    }

    VulkanShaderModule(VulkanShaderModule&& source) noexcept {
        operator=(static_cast<VulkanShaderModule&&>(source));
    }

    VulkanShaderModule& operator=(VulkanShaderModule&& source) noexcept {
        if(this == &source) return *this;
        this->device = source.device;
        this->module = source.module;

        source.module = VK_NULL_HANDLE;

        return *this;
    }

    ~VulkanShaderModule(){
        if(module){
            vkDestroyShaderModule(device, module, nullptr);
        }
    }

    operator VkShaderModule() const {
        return module;
    }

    VkDevice device = VK_NULL_HANDLE;
    VkShaderModule module = VK_NULL_HANDLE;
};