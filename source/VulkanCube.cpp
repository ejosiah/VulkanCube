#include "VulkanCube.h"

void VulkanCube::init() {
    initGlfw();
    initVulkan();
}

void VulkanCube::run() {
    init();
    mainLoop();
    stop();
}

void VulkanCube::mainLoop() {
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        drawFrame();
        std::this_thread::sleep_for(ONE_SECOND);
        vkDeviceWaitIdle(device);
    }
}

void VulkanCube::drawFrame() {
    uint32_t imageIndex;
    auto result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAcquired, VK_NULL_HANDLE, &imageIndex);
    assert(result == VK_SUCCESS);

    VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAcquired.handle;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderingFinished.handle;

    vkQueueSubmit(device.queues.graphics, 1, &submitInfo, VK_NULL_HANDLE);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderingFinished.handle;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain.swapChain;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(device.queues.present, &presentInfo);

}

void VulkanCube::stop() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void VulkanCube::initGlfw(){
    if(!glfwInit()) throw std::runtime_error{"Failed to init GFLW!"};
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWmonitor* monitor = nullptr;
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Cube", monitor, nullptr);

    uint32_t requiredExtensionCount;
    auto requiredExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensionCount);
    instanceExtensionsAndValidationLayers.extensions = std::vector<const char*>(requiredExtensions, requiredExtensions + requiredExtensionCount);

    if constexpr (debugMode){
        instanceExtensionsAndValidationLayers.extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instanceExtensionsAndValidationLayers.validationLayers.push_back("VK_LAYER_KHRONOS_validation");
    }
}

void VulkanCube::initVulkan() {
    createInstance();
    createDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createDevice();
    createSwapChain();
    createRenderPass();
    createFrameBuffer();
    createPipelineLayout();
    createDescriptorPool();
    createGraphicsPipeline();
    createCommandPool();
    createMesh();
    createCamera();
    createDescriptorSet();
    createCommandBuffer();
    createSyncObjects();
}

void VulkanCube::createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.pApplicationName = "Vulkan Cube";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    auto debugInfo = VulkanDebug::debugCreateInfo();
    instance = VulkanInstance{appInfo, instanceExtensionsAndValidationLayers, &debugInfo};
}

void VulkanCube::createDebugMessenger() {
    debug = VulkanDebug{instance};
}

void VulkanCube::createSurface() {
    surface = VulkanSurface{ instance, window};
}

void VulkanCube::pickPhysicalDevice() {
    auto pDevices = enumerate<VkPhysicalDevice>([&](uint32_t* size, VkPhysicalDevice* pDevice){
       return vkEnumeratePhysicalDevices(instance, size, pDevice);
    });

    std::vector<VulkanDevice> devices(pDevices.size());
    std::transform(begin(pDevices), end(pDevices), begin(devices),[&](auto pDevice){
        return VulkanDevice{pDevice};
    });

    std::sort(begin(devices), end(devices), [](auto& a, auto& b){
        return a.score() > b.score();
    });

    device = std::move(devices.front());
    spdlog::info("selected device: {}", device.name());
}

void VulkanCube::createDevice() {
    VkPhysicalDeviceFeatures features{};
    deviceExtensionsAndValidationLayers.extensions.push_back("VK_KHR_swapchain");
    if(device.extensionSupported("VK_KHR_portability_subset")){
        deviceExtensionsAndValidationLayers.extensions.push_back("VK_KHR_portability_subset");
    }
    if constexpr (debugMode){
        // Required for backward compatibility
        deviceExtensionsAndValidationLayers.validationLayers.push_back("VK_LAYER_KHRONOS_validation");
    }
    device.createLogicalDevice(features,
                               deviceExtensionsAndValidationLayers.extensions,
                               deviceExtensionsAndValidationLayers.validationLayers,
                               surface,
                               VK_QUEUE_GRAPHICS_BIT);
}

void VulkanCube::createSwapChain(){
    swapChain = VulkanSwapChain{ device, surface, WIDTH, HEIGHT };
    clearColors.resize(swapChain.imageCount());
    for(int i = 0;i < clearColors.size(); i++){
        clearColors[i].float32[i] = 1.0f;
    }
}


void VulkanCube::createMesh() {
    auto cubeVertices = primitives::cube();
    VkDeviceSize size = sizeof(cubeVertices.vertices[0]) * cubeVertices.vertices.size();

    VulkanBuffer stagingBuffer = device.createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                                     size);

    stagingBuffer.copy(cubeVertices.vertices.data(), size);

    this->cube.vertices = device.createBuffer( VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                               size);

    commandPool.oneTime(device.queues.graphics, [&](VkCommandBuffer commandBuffer){
        VkBufferCopy region{ 0, 0, size };
       vkCmdCopyBuffer(commandBuffer, stagingBuffer, this->cube.vertices, 1, &region);
    });

    size = sizeof(cubeVertices.indices[0]) * cubeVertices.indices.size();
    stagingBuffer = device.createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                                     size);

    this->cube.indices = device.createBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
                                            , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, size);

    commandPool.oneTime(device.queues.graphics, [&](VkCommandBuffer commandBuffer){
       VkBufferCopy region{ 0, 0, size};
        vkCmdCopyBuffer(commandBuffer, stagingBuffer, *this->cube.indices, 1, &region);
    });
}

void VulkanCube::createRenderPass() {
    std::vector<VkAttachmentDescription> attachments;
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChain.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachments.push_back(colorAttachment);

    std::vector<VkAttachmentReference> references(1);
    references[0].attachment = 0;
    references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    std::vector<VkSubpassDescription> subpasses(1);
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = COUNT(references);
    subpasses[0].pColorAttachments = references.data();


    renderPass = VulkanRenderPass{ device, attachments, subpasses};
}

void VulkanCube::createFrameBuffer() {
    framebuffers.resize(swapChain.imageCount());

    for(auto i = 0; i < framebuffers.size(); i++){
        framebuffers[i] = VulkanFramebuffer{ device, renderPass, {swapChain.imageViews[i] }, WIDTH, HEIGHT };
    }
}

void VulkanCube::createPipelineLayout() {
   std::vector<VkDescriptorSetLayoutBinding> bindings;
   VkDescriptorSetLayoutBinding cameraBinding{};
   cameraBinding.binding = 0;
   cameraBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   cameraBinding.descriptorCount = 1;
   cameraBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

   bindings.push_back(cameraBinding);

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = COUNT(bindings);
    createInfo.pBindings = bindings.data();

    vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &descriptorSetLayout);

    pipelineLayout = VulkanPipelineLayout{ device, {descriptorSetLayout} };
}
void VulkanCube::createsPipeline() {


}

void VulkanCube::createDescriptorPool() {
    const auto maxSets = swapChain.imageCount();

    std::vector<VkDescriptorPoolSize> poolSizes{
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}
    };

    descriptorPool = VulkanDescriptorPool{device, maxSets, poolSizes};

}

void VulkanCube::createDescriptorSet() {
    std::vector<VkDescriptorSetLayout> layouts(swapChain.imageCount(), descriptorSetLayout);
    descriptorSets = descriptorPool.allocate(layouts);

    for (auto i = 0; i < descriptorSets.size(); i++) {
        VkWriteDescriptorSet writes{};
        writes.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes.dstSet = descriptorSets[i];
        writes.dstBinding = 0;
        writes.descriptorCount = 1;
        writes.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = camera[i].buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;
        writes.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &writes, 0, nullptr);
    }
}

void VulkanCube::createGraphicsPipeline() {
    auto vertexShaderModule = VulkanShaderModule{ device, "../../resources/shaders/cube.vert.spv"};
    auto fragmentShaderModule = VulkanShaderModule{ device, "../../resources/shaders/cube.frag.spv"};
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = initializers::vertexShaderStages(device, {
            { vertexShaderModule, VK_SHADER_STAGE_VERTEX_BIT},
            { fragmentShaderModule,  VK_SHADER_STAGE_FRAGMENT_BIT}
    });

    auto vertexBindings = Vertex::binding();
    auto attributes = Vertex::attributes();
    VkPipelineVertexInputStateCreateInfo inputState = initializers::vertexInputState(vertexBindings, attributes);
    VkPipelineInputAssemblyStateCreateInfo assemblyState = initializers::inputAssemblyState();
    VkPipelineViewportStateCreateInfo viewportState = initializers::viewportState( initializers::viewport(WIDTH, HEIGHT), initializers::scissor({WIDTH, HEIGHT}));
    VkPipelineRasterizationStateCreateInfo rasterState = initializers::rasterizationState();
    VkPipelineMultisampleStateCreateInfo multisampleState = initializers::multisampleState();
    VkPipelineDepthStencilStateCreateInfo depthStencilState = initializers::depthStencilState();
    VkPipelineColorBlendStateCreateInfo colorBlendState = initializers::colorBlendState();
    VkPipelineDynamicStateCreateInfo  dynamicState = initializers::dynamicState();

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = COUNT(shaderStages);
    pipelineCreateInfo.pStages = shaderStages.data();
    pipelineCreateInfo.pVertexInputState = &inputState;
    pipelineCreateInfo.pInputAssemblyState = &assemblyState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pRasterizationState = &rasterState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    VkPipeline pipeline;
    ASSERT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline));
    graphicsPipeline = VulkanPipeline{device, pipeline};
}

void VulkanCube::createCommandPool() {
    commandPool = VulkanCommandPool{ device, *device.queueFamilyIndex.graphics };
}

void VulkanCube::createCommandBuffer() {
    const uint32_t numCommandBuffers = swapChain.imageCount();
    commandBuffers = commandPool.allocate(numCommandBuffers);

    for(auto i = 0; i < numCommandBuffers; i++){
        auto commandBuffer = commandBuffers[i];
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkClearValue clearValue{};
        clearValue.color = clearColors[i];
        VkClearValue clearValues[1]{ clearValue};

        VkRenderPassBeginInfo beginRenderPass{};
        beginRenderPass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginRenderPass.renderPass = renderPass;
        beginRenderPass.framebuffer = framebuffers[i];
        beginRenderPass.renderArea  = { {0, 0}, {WIDTH, HEIGHT}};
        beginRenderPass.clearValueCount = 1;
        beginRenderPass.pClearValues = clearValues;

        vkCmdBeginRenderPass(commandBuffer, &beginRenderPass, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i].descriptorSet, 0,nullptr);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipeline);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &cube.vertices.buffer, &cube.vertices.size);
        vkCmdBindIndexBuffer(commandBuffer, cube.indices->buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, cube.indices->size/sizeof(uint32_t), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);
        vkEndCommandBuffer(commandBuffer);
    }

}

void VulkanCube::createSyncObjects() {
    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore  semaphore;
    vkCreateSemaphore(device, &createInfo, nullptr, &semaphore);

    imageAcquired = VulkanSemaphore{ device, semaphore};

    vkCreateSemaphore(device, &createInfo, nullptr, &semaphore);
    renderingFinished = VulkanSemaphore{ device, semaphore};
}

void VulkanCube::createCamera() {
    camera.resize(swapChain.imageCount());
    VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    if(!device.supportsMemoryType(flags)){
        flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
    for(auto & cam : camera) {
        cam.buffer = device.createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, flags, Camera::size);
    }
}


