#pragma once

#include "common.h"
#include "VulkanInstance.h"
#include "VulkanDebug.h"
#include "VulkanSurface.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanPipelineLayout.h"
#include "VulkanPipeline.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderPass.h"
#include "Initializers.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDeleters.h"
#include "primitives.h"
#include "VulkanDescriptorSet.h"

template<typename T>
struct Resource : public T{
    static constexpr VkDeviceSize size = sizeof(T);
    VulkanBuffer buffer;

    void flush(){
        T data = *this;
        buffer.copy(data, size);
    }
};


struct mvp{
    glm::mat4 model = glm::mat4(1);
    glm::mat4 view = glm::mat4(1);
    glm::mat4 proj = glm::mat4(1);
};

using Camera = Resource<mvp>;

struct VulkanMesh{
    VulkanBuffer vertices;
    std::optional<VulkanBuffer> indices = {};
    VkDeviceSize size;
};

class VulkanCube{
public:
    void init();

    void run();

    void stop();

protected:
    void initGlfw();

    void mainLoop();

    void drawFrame();

    void initVulkan();

    void pickPhysicalDevice();

    void createDevice();

    void createInstance();

    void createDebugMessenger();

    void createSurface();

    void createSwapChain();

    void createMesh();

    void createRenderPass();

    void createFrameBuffer();

    void createPipelineLayout();

    void createsPipeline();

    void createDescriptorPool();

    void createDescriptorSet();

    void createGraphicsPipeline();

    void createCommandPool();

    void createCommandBuffer();

    void createSyncObjects();

    void createCamera();

protected:
    GLFWwindow* window;
    VulkanInstance instance;
    VulkanDebug debug;
    VulkanSurface surface;
    VulkanDevice device;
    VulkanSwapChain swapChain;
    VulkanRenderPass renderPass;
    VulkanPipelineLayout pipelineLayout;
    VulkanPipeline graphicsPipeline;
    VulkanCommandPool commandPool;
    VulkanDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;

    std::vector<VulkanFramebuffer> framebuffers;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VulkanDescriptorSet> descriptorSets;

    VulkanSemaphore imageAcquired;
    VulkanSemaphore renderingFinished;

    ExtensionsAndValidationLayers instanceExtensionsAndValidationLayers;
    ExtensionsAndValidationLayers deviceExtensionsAndValidationLayers;

    std::vector<VkClearColorValue> clearColors;
    VulkanMesh cube;
    std::vector<Camera> camera;
};