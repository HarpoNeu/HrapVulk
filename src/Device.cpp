#include <device.hpp>

#include <utils.hpp>

#include <math.h>

#include <array>
#include <iostream>
#include <map>
#include <set>

const std::vector<const char*> requestedDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

std::vector<QueueFamily> getPhysicalDeviceQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR& surface)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    std::vector<uint32_t> requestedQueueCounts = {2, 1, 1};
    std::vector<uint32_t> supportedQueueCounts(4);

    for (int i = 0; i < supportedQueueCounts.size(); i++)
    {
        supportedQueueCounts[i] = 0;
    }

    for (VkQueueFamilyProperties queueFamily : queueFamilies)
    {
        supportedQueueCounts[0] += queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT ? 1 : 0;
        supportedQueueCounts[1] += queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT ? 1 : 0;
        supportedQueueCounts[2] += queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT ? 1 : 0;
        supportedQueueCounts[3] += queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT ? 1 : 0;
    }

    std::vector<int> queueFamilyAvailability(queueFamilyCount);
    for (int i = 0; i < queueFamilyCount; i++)
    {
        queueFamilyAvailability[i] = queueFamilies[i].queueCount;
    }

    std::vector<QueueFamily> requestedQueues;

    for (int i = 0; i < requestedQueueCounts.size(); i++)
    {
        std::array<int, 2> queueCountRange = {0, 9999};

        int queueToFind;
        for (int j = 0; j < supportedQueueCounts.size(); j++)
        {
            if (supportedQueueCounts[j] <= queueCountRange[1] && supportedQueueCounts[j] > queueCountRange[0])
            {
                queueToFind = j;
                queueCountRange[1] = supportedQueueCounts[j];
            }
        }

        queueCountRange[0] = supportedQueueCounts[queueToFind];
        supportedQueueCounts[queueToFind] = 99999;

        uint32_t queueFlag;
        switch (queueToFind)
        {
            case 0 : queueFlag = 1; break;
            case 1 : queueFlag = 2; break;
            case 2 : queueFlag = 4; break;
            case 3 : queueFlag = 8; break;
        }

        int queueFamilyToUse = 0;
        int maxAvailableUse = -9999;
        for (int j = 0; j < queueFamilyCount; j++)
        {
            int requestedUse = queueFamilyAvailability[j] - requestedQueueCounts[queueToFind];

            if (requestedUse > maxAvailableUse && queueFamilies[j].queueFlags & queueFlag)
            {
                if ((queueFlag & VK_QUEUE_GRAPHICS_BIT) == queueFlag)
                {
                    VkBool32 presentSupport;
                    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, j, surface, &presentSupport);

                    if (presentSupport != VK_TRUE)
                    {
                        continue;
                    }
                }

                maxAvailableUse = requestedUse;
                queueFamilyToUse = j;
            }
        }

        for (int j = 0; j < requestedQueueCounts[queueToFind]; j++)
        {
            int queueToUse = queueFamilyAvailability[queueFamilyToUse];
            if (queueToUse >= queueFamilies[queueFamilyToUse].queueCount)
            {
                queueToUse = queueToUse % queueFamilies[queueFamilyToUse].queueCount;
            }

            QueueFamily requestedQueue = {};
            requestedQueue.queueFamilyIndex = queueFamilyToUse;
            requestedQueue.queueIndex = queueToUse;
            requestedQueue.queueType = queueToFind;

            requestedQueues.push_back(requestedQueue);

            queueFamilyAvailability[queueFamilyToUse]--;
        }
    }

    return requestedQueues;
}

bool deviceExtensionsSupported(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> deviceExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, deviceExtensions.data());

    std::set<std::string> requiredExtensions(requestedDeviceExtensions.begin(), requestedDeviceExtensions.end());

    for (const auto& extension : deviceExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapchainSupportDetails querySwapchainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR& surface)
{
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

int rateDevice(VkPhysicalDevice device, VkSurfaceKHR& surface)
{
    auto queueFamilies = getPhysicalDeviceQueueFamilies(device, surface);
    bool extensionsSupported = deviceExtensionsSupported(device);

    int rating = 0;

    std::set<uint32_t> uniqueQueues;
    for (auto queueFamily : queueFamilies)
    {
        uint32_t queueValue = (queueFamily.queueFamilyIndex * 16) + (queueFamily.queueIndex);
        uniqueQueues.insert(queueValue);
    }

    rating += uniqueQueues.size();

    if (!extensionsSupported)
    {
        rating = -1;
    }
    else
    {
        SwapchainSupportDetails swapchainDetails = querySwapchainSupportDetails(device, surface);
        if (swapchainDetails.formats.empty() || swapchainDetails.presentModes.empty())
        {
            rating = -1;
        }
    }

    return rating;
}

VkPhysicalDevice selectPhysicalDevice(VkSurfaceKHR& surface)
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(getInstance(), &physicalDeviceCount, nullptr);

    if (physicalDeviceCount == 0)
    {
        throw std::runtime_error("Error! No physical devices found!");
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(getInstance(), &physicalDeviceCount, physicalDevices.data());

    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
    int bestRating = -1;
    for (const auto& physicalDevice : physicalDevices)
    {
        int deviceRating = rateDevice(physicalDevice, surface);
        if (deviceRating > bestRating)
        {
            bestDevice = physicalDevice;
            bestRating = deviceRating;
        }
    }

    return bestDevice;
}

Device::Device()
{

}

Device::~Device()
{

}

void Device::create(VkSurfaceKHR& surface)
{
    const std::vector<const char*> debugLayers {
        "VK_LAYER_KHRONOS_validation"
    };

    physicalDevice = selectPhysicalDevice(surface);

    here();

    auto queueFamilies = getPhysicalDeviceQueueFamilies(physicalDevice, surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::map<uint32_t, uint32_t> uniqueQueueFamilies;
    for (auto queueFamily : queueFamilies)
    {
        uniqueQueueFamilies.insert(std::pair<uint32_t, uint32_t>(queueFamily.queueFamilyIndex, 0));
    }
    for (auto queueFamily : queueFamilies)
    {
        uniqueQueueFamilies.find(queueFamily.queueFamilyIndex)->second++;
    }

    float queuePriority = 1.0f;
    for (std::pair queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily.first;
        queueCreateInfo.queueCount = queueFamily.second;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requestedDeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = requestedDeviceExtensions.data();
    deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(debugLayers.size());
    deviceCreateInfo.ppEnabledLayerNames = debugLayers.data();

    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("Error! Failed to create device!");
    }

    for (auto queueFamily : queueFamilies)
    {
        VkQueue vQueue;
        vkGetDeviceQueue(device, queueFamily.queueFamilyIndex, queueFamily.queueIndex, &vQueue);

        Queue queue = {vQueue, queueFamily};
        
        switch (queueFamily.queueType)
        {
           case 0: graphicsQueues.push_back(queue); break;
           case 1: transferQueues.push_back(queue); break;
           case 2: computeQueues.push_back(queue); break;
           default: break;
        }
    }

    for (std::map<uint32_t, uint32_t>::iterator it = uniqueQueueFamilies.begin(); it != uniqueQueueFamilies.end(); ++it)
    {
        VkCommandPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.queueFamilyIndex = it->first;
        poolCreateInfo.flags = 0;

        VkCommandPool pool;
        if (createCommandPool(&poolCreateInfo, nullptr, &pool) != VK_SUCCESS)
        {
            throw std::runtime_error("Error! Failed to create command pool!");
        }

        commandPools.insert(std::make_pair(it->first, pool));
    }
}

void Device::destroy()
{
    for (std::map<uint32_t, VkCommandPool>::iterator it = commandPools.begin(); it != commandPools.end(); ++it)
    {
        vkDestroyCommandPool(device, it->second, nullptr);
    }

    vkDestroyDevice(device, nullptr);
}

VkResult Device::createBuffer(VkBufferCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer)
{
    return vkCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
}

void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory)
{
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (createBuffer(&bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Error! Failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    getBufferMemoryRequirements(buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (allocateMemory(&allocInfo, nullptr, &memory) != VK_SUCCESS)
    {
        throw std::runtime_error("Error! Failed to allocate buffer memory!");
    }

    bindBufferMemory(buffer, memory, 0);
}

VkResult Device::createCommandPool(VkCommandPoolCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkCommandPool* pPool)
{
    return vkCreateCommandPool(device, pCreateInfo, pAllocator, pPool);
}

VkResult Device::createDescriptorPool(VkDescriptorPoolCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkDescriptorPool* pPool)
{
    return vkCreateDescriptorPool(device, pCreateInfo, nullptr, pPool);
}

VkResult Device::createDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pLayout)
{
    return vkCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pLayout);
}

VkResult Device::createFence(VkFenceCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkFence* pFence)
{
    return vkCreateFence(device, pCreateInfo, pAllocator, pFence);
}

VkResult Device::createFramebuffer(VkFramebufferCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer)
{
    return vkCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
}

VkResult Device::createGraphicsPipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    return vkCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

VkResult Device::createImageView(VkImageViewCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkImageView* pImageView)
{
    return vkCreateImageView(device, pCreateInfo, pAllocator, pImageView);
}

VkResult Device::createPipelineLayout(VkPipelineLayoutCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkPipelineLayout* pLayout)
{
    return vkCreatePipelineLayout(device, pCreateInfo, nullptr, pLayout);
}

VkResult Device::createRenderPass(VkRenderPassCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass)
{
    return vkCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
}

VkResult Device::createSemaphore(VkSemaphoreCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore)
{
    return vkCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
}

VkResult Device::createShaderModule(VkShaderModuleCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkShaderModule* pModule)
{
    return vkCreateShaderModule(device, pCreateInfo, pAllocator, pModule);
}

VkResult Device::createSwapchain(VkSwapchainCreateInfoKHR* pCreateInfo, VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain)
{
    return vkCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
}

VkResult Device::allocateMemory(VkMemoryAllocateInfo* pAllocInfo, VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory)
{
    return vkAllocateMemory(device, pAllocInfo, pAllocator, pMemory);
}

VkResult Device::bindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize offset)
{
    return vkBindBufferMemory(device, buffer, memory, offset);
}

void Device::freeMemory(VkDeviceMemory memory, VkAllocationCallbacks* pAllocator)
{
    vkFreeMemory(device, memory, pAllocator);
}

VkResult Device::allocateCommandBuffers(VkCommandBufferAllocateInfo* pAllocInfo, VkCommandBuffer* pBuffers)
{
    return vkAllocateCommandBuffers(device, pAllocInfo, pBuffers);
}

void Device::freeCommandBuffers(VkCommandPool pool, uint32_t bufferCount, VkCommandBuffer* pBuffers)
{
    vkFreeCommandBuffers(device, pool, bufferCount, pBuffers);
}

VkResult Device::allocateDescriptorSets(VkDescriptorSetAllocateInfo* pAllocInfo, VkDescriptorSet* pSets)
{
    return vkAllocateDescriptorSets(device, pAllocInfo, pSets);
}

void Device::updateDescriptorSets(uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pWriteSets, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pCopySets)
{
    vkUpdateDescriptorSets(device, descriptorWriteCount, pWriteSets, descriptorCopyCount, pCopySets);
}

void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool pool, VkQueue queue)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(pool);

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer, pool, queue);
}

void Device::destroyBuffer(VkBuffer buffer, VkAllocationCallbacks* pAllocator)
{
    vkDestroyBuffer(device, buffer, pAllocator);
}

void Device::destroyCommandPool(VkCommandPool pool, VkAllocationCallbacks* pAllocator)
{
    vkDestroyCommandPool(device, pool, pAllocator);
}

void Device::destroyDescriptorPool(VkDescriptorPool pool, VkAllocationCallbacks* pAllocator)
{
    vkDestroyDescriptorPool(device, pool, pAllocator);
}

void Device::destroyDescriptorSetLayout(VkDescriptorSetLayout layout, VkAllocationCallbacks* pAllocator)
{
    vkDestroyDescriptorSetLayout(device, layout, nullptr);
}

void Device::destroyFence(VkFence fence, VkAllocationCallbacks* pAllocator)
{
    vkDestroyFence(device, fence, nullptr);
}

void Device::destroyFramebuffer(VkFramebuffer framebuffer, VkAllocationCallbacks* pAllocator)
{
    vkDestroyFramebuffer(device, framebuffer, pAllocator);
}

void Device::destroyImageView(VkImageView view, VkAllocationCallbacks* pAllocator)
{
    vkDestroyImageView(device, view, pAllocator);
}

void Device::destroyPipeline(VkPipeline pipeline, VkAllocationCallbacks* pAllocator)
{
    vkDestroyPipeline(device, pipeline, pAllocator);
}

void Device::destroyPipelineLayout(VkPipelineLayout layout, VkAllocationCallbacks* pAllocator)
{
    vkDestroyPipelineLayout(device, layout, pAllocator);
}

void Device::destroyRenderPass(VkRenderPass renderPass, VkAllocationCallbacks* pAllocator)
{
    vkDestroyRenderPass(device, renderPass, pAllocator);
}

void Device::destroySemaphore(VkSemaphore semaphore, VkAllocationCallbacks* pAllocator)
{
    vkDestroySemaphore(device, semaphore, pAllocator);
}

void Device::destroyShaderModule(VkShaderModule module, VkAllocationCallbacks* pAllocator)
{
    vkDestroyShaderModule(device, module, pAllocator);
}

void Device::destroySwapchain(VkSwapchainKHR swapchain, VkAllocationCallbacks* pAllocator)
{
    vkDestroySwapchainKHR(device, swapchain, pAllocator);
}

VkResult Device::mapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)
{
    return vkMapMemory(device, memory, offset, size, flags, ppData);
}

void Device::unmapMemory(VkDeviceMemory memory)
{
    vkUnmapMemory(device, memory);
}

VkCommandBuffer Device::beginSingleTimeCommands(VkCommandPool pool)
{
    VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
    commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocInfo.commandPool = pool;
    commandBufferAllocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    allocateCommandBuffers(&commandBufferAllocInfo, &commandBuffer);

    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    return commandBuffer;
}

void Device::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool pool, VkQueue queue)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    freeCommandBuffers(pool, 1, &commandBuffer);
}

VkResult Device::waitForFences(uint32_t fenceCount, VkFence* pFences, VkBool32 waitAll, uint64_t timeout)
{
    return vkWaitForFences(device, fenceCount, pFences, waitAll, timeout);
}

VkResult Device::resetFences(uint32_t fenceCount, VkFence* pFences)
{
    return vkResetFences(device, fenceCount, pFences);
}

VkResult Device::waitIdle()
{
    return vkDeviceWaitIdle(device);
}

int Device::getRating(VkSurfaceKHR& surface)
{
    return rateDevice(physicalDevice, surface);
}

void Device::getBufferMemoryRequirements(VkBuffer buffer, VkMemoryRequirements* pRequirements)
{
    vkGetBufferMemoryRequirements(device, buffer, pRequirements);
}

void Device::getPhysicalDeviceMemoryProperties(VkPhysicalDeviceMemoryProperties* pProperties)
{
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, pProperties);
}  

VkResult Device::getSwapchainImages(VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages)
{
    return vkGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
}

std::vector<QueueFamily> Device::getQueueFamilies(VkSurfaceKHR& surface)
{
    auto queueFamilies = getPhysicalDeviceQueueFamilies(physicalDevice, surface);
    return queueFamilies;
}

VkBool32 Device::getPhysicalDeviceSurfaceSupport(VkSurfaceKHR& surface)
{
    VkBool32 supported;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, surface, &supported);
    return supported;
}

SwapchainSupportDetails Device::getSwapchainSupportDetails(VkSurfaceKHR& surface)
{
    return querySwapchainSupportDetails(physicalDevice, surface);
}

std::vector<Queue>& Device::getGraphicsQueues()
{
    return graphicsQueues;
}

VkCommandPool Device::getCommandPool(Queue queue)
{
    uint32_t queueFamily = queue.family.queueFamilyIndex;
    VkCommandPool pool = commandPools.find(queueFamily)->second;
    return pool;
}

uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    getPhysicalDeviceMemoryProperties(&memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if (typeFilter & (i << 1) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Error! Failed to find suitable memory type!");
}

VkResult Device::acquireNextImageKHR(VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex)
{
    return vkAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
}