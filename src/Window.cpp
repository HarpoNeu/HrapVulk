#include <window.hpp>
#include <utils.hpp>
#include <device.hpp>
#include <uniform.hpp>

#include <array>
#include <chrono>
#include <iostream>
#include <set>
#include <stdexcept>
#include <sstream>

#include <unistd.h>

const int MAX_FRAMES_IN_FLIGHT = 2;

Window::Window()
{
    window = nullptr;

    dimensions.w = 100;
    dimensions.h = 100;
    title = "Application";
    
    currentFrame = 0;

    launched = false;
    shown = false;
}

Window::~Window()
{

}

void Window::setWidth(int width)
{
    this->dimensions.w = width;

    if (window != nullptr)
    {
        glfwSetWindowSize(window, this->dimensions.w, this->dimensions.h);
    }
}

void Window::setHeight(int height)
{
    this->dimensions.h = height;

    if (window != nullptr)
    {
        glfwSetWindowSize(window, this->dimensions.w, this->dimensions.h);
    }
}

void Window::setSize(int width, int height)
{
    this->dimensions.w = width;
    this->dimensions.h = height;

    if (window != nullptr)
    {
        glfwSetWindowSize(window, this->dimensions.w, this->dimensions.h);
    }
}

void Window::setTitle(char* title)
{
    this->title = title;

    if (window != nullptr)
    {
        glfwSetWindowTitle(window, title);
    }
}

void Window::show() 
{
    shown = true;
}

void Window::hide()
{
    if (window == nullptr)
    {
        return;
    }

    glfwHideWindow(window);

    destroySwapchain();
}

void Window::launch(std::vector<Device>& devices)
{
    if (!launched)
    {
        launched = true;

        window = glfwCreateWindow(dimensions.w, dimensions.h, title, nullptr, nullptr);
        if (glfwCreateWindowSurface(getInstance(), window, nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("Error! Failed to create window surface!");
        }

        std::cout << surface << std::endl;

        if (devices.empty())
        {
            device.create(surface);
            devices.push_back(device);
        }
        else
        {
            int bestRating = 0;
            Device bestDevice;

            for (Device device : devices)
            {
                if (device.getRating(surface) > bestRating)
                {
                    bestDevice = device;
                }
            }

            if (bestRating <= 0)
            {
                device.create(surface);
            }
            else
            {
                device = bestDevice;
            }
        }
        
        if (device.getPhysicalDeviceSurfaceSupport(surface) != VK_TRUE)
        {
            throw std::runtime_error("Error! Surface not supported by device");
        }

        createSwapchain();
        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createFramebuffers();
        createVertexBuffers();
        createIndexBuffers();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
        createSyncObjects();
    }

    glfwShowWindow(window);
}

void Window::drawFrame()
{
    uint32_t imageIndex;
    VkResult result = device.acquireNextImageKHR(swapchain.swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Error! Failed to acquire swapchain image!");
    }

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        device.waitForFences(1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }

    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformObject ubo = {};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), swapchain.extent.width / (float) swapchain.extent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    void* data;
    device.mapMemory(uniformBufferMemories[imageIndex], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    device.unmapMemory(uniformBufferMemories[imageIndex]);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    device.waitForFences(1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    device.resetFences(1, &inFlightFences[currentFrame]);

    Queue queue = device.getGraphicsQueues()[0];
    if (vkQueueSubmit(queue.queue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Error! Failed to submit to queue!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapchains[] = {swapchain.swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    presentInfo.pResults = nullptr;

    Queue presentQueue = device.getGraphicsQueues()[1];

    result = vkQueuePresentKHR(presentQueue.queue, &presentInfo);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Error! Failed to present swapchain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Window::destroy()
{
    device.waitIdle();

    destroySwapchain();

    device.destroyDescriptorSetLayout(descriptorSetLayout, nullptr);

    for (VkDeviceMemory memory : vertexBufferMemories)
    {
        device.freeMemory(memory, nullptr);
    }
    for (VkDeviceMemory memory : indexBufferMemories)
    {
        device.freeMemory(memory, nullptr);
    }

    for (VkBuffer buffer : vertexBuffers)
    {
        device.destroyBuffer(buffer, nullptr);
    }
    for (VkBuffer buffer : indexBuffers)
    {
        device.destroyBuffer(buffer, nullptr);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        device.destroySemaphore(imageAvailableSemaphores[i], nullptr);
        device.destroySemaphore(renderFinishedSemaphores[i], nullptr);
        device.destroyFence(inFlightFences[i], nullptr);
    }

    vkDestroySurfaceKHR(getInstance(), surface, nullptr);

    glfwDestroyWindow(window);
    window = nullptr;
}

void Window::addShape(Shape* shape)
{
    shapes.push_back(shape);
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(window);
}

VkSurfaceKHR Window::getSurface()
{
    return surface;
}

void Window::createSwapchain()
{
    SwapchainSupportDetails swapchainSupportDetails = device.getSwapchainSupportDetails(surface);

    VkSurfaceFormatKHR surfaceFormat = swapchainSupportDetails.formats[0];
    for (const auto& availableFormat : swapchainSupportDetails.formats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surfaceFormat = availableFormat;
            break;
        }
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& availablePresentMode : swapchainSupportDetails.presentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = availablePresentMode;
            break;
        }
    }

    VkExtent2D extent;
    if (swapchainSupportDetails.capabilities.currentExtent.width != UINT32_MAX)
    {
        extent = swapchainSupportDetails.capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::max(
            swapchainSupportDetails.capabilities.minImageExtent.width, 
            std::min(
                swapchainSupportDetails.capabilities.maxImageExtent.width, 
                actualExtent.width
        ));
        actualExtent.height = std::max(
            swapchainSupportDetails.capabilities.minImageExtent.height, 
            std::min(
                swapchainSupportDetails.capabilities.maxImageExtent.height, 
                actualExtent.height
        ));

        extent = actualExtent;
    }

    uint32_t imageCount = swapchainSupportDetails.capabilities.minImageCount + 1;

    if (swapchainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapchainSupportDetails.capabilities.maxImageCount)
    {
        imageCount = swapchainSupportDetails.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto graphicsQueues = device.getGraphicsQueues();
    std::set<uint32_t> uniqueQueueFamilies;
    for (auto& graphicsQueue : graphicsQueues)
    {
        uniqueQueueFamilies.insert(graphicsQueue.family.queueFamilyIndex);
    }
    std::vector<uint32_t> vQueueFamilies(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end());
    if (uniqueQueueFamilies.size() > 1)
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(vQueueFamilies.size());
        swapchainCreateInfo.pQueueFamilyIndices = vQueueFamilies.data();
    }
    else
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapchainCreateInfo.preTransform = swapchainSupportDetails.capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (device.createSwapchain(&swapchainCreateInfo, nullptr, &swapchain.swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("Error! Failed to create swapchain!");
    }

    device.getSwapchainImages(swapchain.swapchain, &imageCount, nullptr);
    swapchain.images.resize(imageCount);
    device.getSwapchainImages(swapchain.swapchain, &imageCount, swapchain.images.data());

    swapchain.format = surfaceFormat.format;
    swapchain.extent = extent;

    swapchain.imageViews.resize(swapchain.images.size());

    for (size_t i = 0; i < swapchain.images.size(); i++)
    {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = swapchain.images[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = swapchain.format;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        if (device.createImageView(&imageViewCreateInfo, nullptr, &swapchain.imageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Error! Failed to create image view!");
        }
    }
}

void Window::createRenderPass()
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapchain.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pResolveAttachments = nullptr;

    VkSubpassDependency subpassDependency = {};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = 0;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};
    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassCreateInfo.pAttachments = attachments.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;

    if (device.createRenderPass(&renderPassCreateInfo, nullptr, &pipeline.renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Error! Failed to create render pass!");
    }
}

void Window::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = 1;
    layoutCreateInfo.pBindings = &uboLayoutBinding;

    if (device.createDescriptorSetLayout(&layoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Error! Failed to create descriptor set layout!");
    }
}

void Window::createGraphicsPipeline()
{
    auto vertShaderCode = readFile("/build/resources/shaders/uvertex.spv");
    auto fragShaderCode = readFile("/build/resources/shaders/fragment.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo = {};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.module = vertShaderModule;
    vertShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo = {};
    fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageCreateInfo.module = fragShaderModule;
    fragShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapchain.extent.width;
    viewport.height = (float) swapchain.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapchain.extent;

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.pViewports = &viewport;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo.lineWidth = 1.0f;
    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
    rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
    rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleStateCreateInfo.minSampleShading = 1.0f;
    multisampleStateCreateInfo.pSampleMask = nullptr;
    multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentState.blendEnable = VK_FALSE;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
    colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
    colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

    VkPipelineDepthStencilStateCreateInfo depthStencilStageCreateInfo = {};
    depthStencilStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStageCreateInfo.depthWriteEnable = VK_TRUE;
    depthStencilStageCreateInfo.depthTestEnable = VK_TRUE;
    depthStencilStageCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilStageCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilStageCreateInfo.minDepthBounds = 0.0f;
    depthStencilStageCreateInfo.maxDepthBounds = 1.0f;
    depthStencilStageCreateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStageCreateInfo.front = {};
    depthStencilStageCreateInfo.back = {};

    VkPipelineLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.setLayoutCount = 1;
    layoutCreateInfo.pSetLayouts = &descriptorSetLayout;
    layoutCreateInfo.pushConstantRangeCount = 0;
    layoutCreateInfo.pPushConstantRanges = nullptr;

    if (device.createPipelineLayout(&layoutCreateInfo, nullptr, &pipeline.layout) != VK_SUCCESS)
    {
        throw std::runtime_error("Error! Failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilStageCreateInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
    pipelineCreateInfo.pDynamicState = nullptr;
    pipelineCreateInfo.layout = pipeline.layout;
    pipelineCreateInfo.renderPass = pipeline.renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    if (device.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline.pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Error! Failed to create graphics pipeline!");
    }

    device.destroyShaderModule(vertShaderModule, nullptr);
    device.destroyShaderModule(fragShaderModule, nullptr);
}

void Window::createFramebuffers()
{
    swapchain.framebuffers.resize(swapchain.imageViews.size());

    for (size_t i = 0; i < swapchain.framebuffers.size(); i++)
    {
        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = pipeline.renderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = &swapchain.imageViews[i];
        framebufferCreateInfo.width = swapchain.extent.width;
        framebufferCreateInfo.height = swapchain.extent.height;
        framebufferCreateInfo.layers = 1;

        if (device.createFramebuffer(&framebufferCreateInfo, nullptr, &swapchain.framebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Error! Failed to create framebuffer");
        }
    }
}

void Window::createVertexBuffers()
{
    vertexBuffers.resize(shapes.size());
    vertexBufferMemories.resize(shapes.size());

    for (size_t i = 0; i < shapes.size(); i++)
    {
        std::vector<Vertex> vertices;

        switch(shapes[i]->getType())
        {
            case SHAPE_TYPE_RECT:
                vertices = Rect::getVertices(static_cast<Rect*>(shapes[i]), dimensions);
                std::cout << vertices[0].color.r << ", " << vertices[0].color.g << ", " << vertices[0].color.b << std::endl;
                break;
            default:
                vertices = {};
                break;
        }

        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        device.mapMemory(stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
        device.unmapMemory(stagingBufferMemory);

        Queue queue = device.getGraphicsQueues()[0];
        VkCommandPool commandPool = device.getCommandPool(queue);

        device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffers[i], vertexBufferMemories[i]);
        device.copyBuffer(stagingBuffer, vertexBuffers[i], bufferSize, commandPool, queue.queue);

        device.destroyBuffer(stagingBuffer, nullptr);
        device.freeMemory(stagingBufferMemory, nullptr);
    }
}

void Window::createIndexBuffers()
{
    indexBuffers.resize(shapes.size());
    indexBufferMemories.resize(shapes.size());

    for (size_t i = 0; i < shapes.size(); i++)
    {
        std::vector<uint16_t> indices;

        switch(shapes[i]->getType())
        {
            case SHAPE_TYPE_RECT:
                indices = Rect::getIndices();
                break;
            default:
                indices = {};
                break;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        device.mapMemory(stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t) bufferSize);
        device.unmapMemory(stagingBufferMemory);

        Queue queue = device.getGraphicsQueues()[0];
        VkCommandPool commandPool = device.getCommandPool(queue);

        device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffers[i], indexBufferMemories[i]);
        device.copyBuffer(stagingBuffer, indexBuffers[i], bufferSize, commandPool, queue.queue);

        device.destroyBuffer(stagingBuffer, nullptr);
        device.freeMemory(stagingBufferMemory, nullptr);
    }
}

void Window::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformObject);

    uniformBuffers.resize(swapchain.images.size());
    uniformBufferMemories.resize(swapchain.images.size());

    for (size_t i = 0; i < swapchain.images.size(); i++)
    {
        device.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, uniformBuffers[i], uniformBufferMemories[i]);
    }
}

void Window::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(swapchain.images.size());

    VkDescriptorPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount = 1;
    poolCreateInfo.pPoolSizes = &poolSize;
    poolCreateInfo.maxSets = static_cast<uint32_t>(swapchain.images.size());

    if (device.createDescriptorPool(&poolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Error! Failed to create descriptor pool!");
    }
}

void Window::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(swapchain.images.size(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = static_cast<uint32_t>(swapchain.images.size());
    descriptorSetAllocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(swapchain.images.size());

    if (device.allocateDescriptorSets(&descriptorSetAllocInfo, descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Error! Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < swapchain.images.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformObject);

        VkWriteDescriptorSet writeSet = {};
        writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeSet.dstSet = descriptorSets[i];
        writeSet.dstBinding = 0;
        writeSet.dstArrayElement = 0;
        writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeSet.descriptorCount = 1;
        writeSet.pBufferInfo = &bufferInfo;
        writeSet.pImageInfo = nullptr;
        writeSet.pTexelBufferView = nullptr;

        device.updateDescriptorSets(1, &writeSet, 0, nullptr);
    }
}

void Window::createCommandBuffers()
{
    Queue queue = device.getGraphicsQueues()[0];
    VkCommandPool commandPool = device.getCommandPool(queue);

    commandBuffers.resize(swapchain.framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    if (device.allocateCommandBuffers(&allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Error! Failed to allocate command buffers!");
    }

    for (size_t i = 0; i < commandBuffers.size(); i++)
    {
        std::vector<std::vector<uint16_t>> indices;
        for (size_t j = 0; j < shapes.size(); j++)
        {
            switch(shapes[j]->getType())
            {
                case SHAPE_TYPE_RECT:
                    indices.push_back(Rect::getIndices());
                    break;
                default:
                    indices.push_back({});
                    break;
            }
        }

        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Error! Failed to begin command buffers!");
        }

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = pipeline.renderPass;
        renderPassBeginInfo.framebuffer = swapchain.framebuffers[i];
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = swapchain.extent;

        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

        VkDeviceSize offsets[] = {0};
        for (size_t j = 0; j < shapes.size(); j++)
        {
            vkCmdBindIndexBuffer(commandBuffers[i], indexBuffers[j], 0, VK_INDEX_TYPE_UINT16);
            vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffers[j], offsets);
            vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &descriptorSets[i], 0, nullptr);
            vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices[j].size()), 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(commandBuffers[i]);

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Error! Failed to end command buffer!");
        }
    }
}

void Window::createSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapchain.images.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (device.createSemaphore(&semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
         || device.createSemaphore(&semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
         || device.createFence(&fenceCreateInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Error! Failed to create sync objects!");
        }
    }
}

void Window::destroySwapchain()
{
    for (auto framebuffer : swapchain.framebuffers)
    {
        device.destroyFramebuffer(framebuffer, nullptr);
    }

    Queue commandBufferQueue = device.getGraphicsQueues()[0];
    VkCommandPool commandPool = device.getCommandPool(commandBufferQueue);

    device.freeCommandBuffers(commandPool, (uint32_t) commandBuffers.size(), commandBuffers.data());

    device.destroyPipeline(pipeline.pipeline, nullptr);

    device.destroyPipelineLayout(pipeline.layout, nullptr);

    device.destroyRenderPass(pipeline.renderPass, nullptr);

    for (auto imageView : swapchain.imageViews)
    {
        device.destroyImageView(imageView, nullptr);
    }

    device.destroySwapchain(swapchain.swapchain, nullptr);

    for (size_t i = 0; i < uniformBuffers.size(); i++)
    {
        device.destroyBuffer(uniformBuffers[i], nullptr);
        device.freeMemory(uniformBufferMemories[i], nullptr);
    }

    device.destroyDescriptorPool(descriptorPool, nullptr);
}

VkShaderModule Window::createShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo moduleCreateInfo = {};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.codeSize = code.size();
    moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule module;
    if (device.createShaderModule(&moduleCreateInfo, nullptr, &module) != VK_SUCCESS)
    {
        throw std::runtime_error("Error! Failed to create shader module!");
    }
    return module;
}

void createNewDevice(Window& window, Device* device)
{

}