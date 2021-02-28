#pragma once
#include "common.h"

struct ExtensionsAndValidationLayers{
    std::vector<const char*> extensions;
    std::vector<const char*> validationLayers;
};

struct VulkanInstance{

    VulkanInstance() = default;

    inline VulkanInstance(const VkApplicationInfo& appInfo, const ExtensionsAndValidationLayers& extAndValidationLayers, void* next = nullptr){
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pNext = next;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = extAndValidationLayers.extensions.size();
        createInfo.ppEnabledExtensionNames = extAndValidationLayers.extensions.data();
        createInfo.enabledLayerCount = extAndValidationLayers.validationLayers.size();
        createInfo.ppEnabledLayerNames = extAndValidationLayers.validationLayers.data();

        auto res = vkCreateInstance(&createInfo, nullptr, &instance);
        if(res != VK_SUCCESS) throw std::runtime_error{"Failed to create Instance"};

    }

    VulkanInstance(const VulkanInstance&) = delete;

    VulkanInstance(VulkanInstance&& source) noexcept {
        operator=(static_cast<VulkanInstance&&>(source));
    }

    VulkanInstance& operator=(const VulkanInstance&) = delete;

    VulkanInstance& operator=(VulkanInstance&& source) noexcept {
        this->instance = source.instance;
        source.instance = VK_NULL_HANDLE;
        return *this;
    }

    ~VulkanInstance(){
        if(instance) {
            vkDestroyInstance(instance, nullptr);
        }
    }

    operator VkInstance() const noexcept{
        return instance;
    }

    static std::vector<VkExtensionProperties> getExtensions(cstring layerName = nullptr) noexcept{
        return enumerate<VkExtensionProperties>([&](uint32_t *size, VkExtensionProperties *props) {
            return vkEnumerateInstanceExtensionProperties(layerName, size, props);
        });
    }

    static std::vector<VkLayerProperties> getLayers() noexcept{
         return enumerate<VkLayerProperties>([](uint32_t *size, VkLayerProperties *props) {
             return vkEnumerateInstanceLayerProperties(size, props);
         });
    }

    static bool extensionSupported(const char* extension) noexcept {
       return std::any_of(begin(extensions), end(extensions), [&](auto& ext){
          return strcmp(ext.extensionName, extension) == 0;
       });
    }

    static bool layerSupported(const char* layerName) noexcept{
        return std::any_of(begin(layers), end(layers), [&](auto& layer){
            return strcmp(layer.layerName, layerName) == 0;
        });
    }

    VkInstance instance = VK_NULL_HANDLE;

   static std::vector<VkExtensionProperties> extensions;
   static std::vector<VkLayerProperties> layers;
};

