#pragma once

#include <vulkan/vulkan.h>

#include <map>
#include <vector>

struct QueueFamily
{
    uint32_t queueFamilyIndex;
    uint32_t queueIndex;
    int queueType;
};

struct Queue
{
    VkQueue queue;
    QueueFamily family;
};

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Device
{
public:

    Device();
    ~Device();

    void create();
    void destroy();

    VkResult createBuffer(VkBufferCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer);
    VkResult createCommandPool(VkCommandPoolCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkCommandPool* pPool);
    VkResult createFence(VkFenceCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkFence* pFence);
    VkResult createFramebuffer(VkFramebufferCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer);
    VkResult createGraphicsPipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
    VkResult createImageView(VkImageViewCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkImageView* pImageView);
    VkResult createPipelineLayout(VkPipelineLayoutCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkPipelineLayout* pLayout);
    VkResult createRenderPass(VkRenderPassCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);
    VkResult createSemaphore(VkSemaphoreCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore);
    VkResult createShaderModule(VkShaderModuleCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkShaderModule* pModule);
    VkResult createSwapchain(VkSwapchainCreateInfoKHR* pCreateInfo, VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);

    VkResult allocateMemory(VkMemoryAllocateInfo* pAllocInfo, VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory);
    VkResult bindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize offset);
    void freeMemory(VkDeviceMemory memory, VkAllocationCallbacks* pAllocator);

    VkResult allocateCommandBuffers(VkCommandBufferAllocateInfo* pAllocInfo, VkCommandBuffer* pBuffers);
    void freeCommandBuffers(VkCommandPool pool, uint32_t bufferCount, VkCommandBuffer* pBuffers);

    void destroyBuffer(VkBuffer buffer, VkAllocationCallbacks* pAllocator);
    void destroyCommandPool(VkCommandPool pool, VkAllocationCallbacks* pAllocator);
    void destroyFence(VkFence fence, VkAllocationCallbacks* pAllocator);
    void destroyFramebuffer(VkFramebuffer framebuffer, VkAllocationCallbacks* pAllocator);
    void destroyImageView(VkImageView view, VkAllocationCallbacks* pAllocator);
    void destroyRenderPass(VkRenderPass renderPass, VkAllocationCallbacks* pAllocator);
    void destroyPipeline(VkPipeline pipeline, VkAllocationCallbacks* pAllocator);
    void destroyPipelineLayout(VkPipelineLayout layout, VkAllocationCallbacks* pAllocator);
    void destroySemaphore(VkSemaphore semaphore, VkAllocationCallbacks* pAllocator);
    void destroyShaderModule(VkShaderModule module, VkAllocationCallbacks* pAllocator);
    void destroySwapchain(VkSwapchainKHR swapchain, VkAllocationCallbacks* pAllocator);

    VkResult mapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData);
    void unmapMemory(VkDeviceMemory memory);

    VkResult waitForFences(uint32_t fenceCount, VkFence* pFences, VkBool32 waitAll, uint64_t timeout);
    VkResult resetFences(uint32_t fenceCount, VkFence* pFences);

    VkResult waitIdle();

    void getBufferMemoryRequirements(VkBuffer buffer, VkMemoryRequirements* pRequirements);
    void getPhysicalDeviceMemoryProperties(VkPhysicalDeviceMemoryProperties* pProperties);

    VkResult getSwapchainImages(VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages);

    std::vector<QueueFamily> getQueueFamilies();
    VkBool32 getPhysicalDeviceSurfaceSupport(VkSurfaceKHR& surface);
    SwapchainSupportDetails getSwapchainSupportDetails(VkSurfaceKHR& surface);
    std::vector<Queue>& getGraphicsQueues();
    VkCommandPool getCommandPool(Queue queue);
    VkResult acquireNextImageKHR(VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);

protected:



private:

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    std::vector<Queue> graphicsQueues;
    std::vector<Queue> transferQueues;
    std::vector<Queue> computeQueues;

    std::map<uint32_t, VkCommandPool> commandPools;

};