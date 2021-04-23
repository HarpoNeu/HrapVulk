#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <vector>

#include <device.hpp>
#include <shape.hpp>

struct Swapchain
{
    VkSwapchainKHR swapchain;
    VkFormat format;
    VkExtent2D extent;

    std::vector<VkImageView> imageViews;
    std::vector<VkImage> images;
    std::vector<VkFramebuffer> framebuffers;
};

struct GraphicsPipeline
{
    VkRenderPass renderPass;
    VkPipelineLayout layout;
    VkPipeline pipeline;

    std::vector<VkShaderModule> shaderModules;
};

class Window
{
public:

    /*! @brief Default constructor for 'Window' class
     *  
     */
    Window();
    /*! @brief Default destructor for 'Window' class
     * 
     */
    ~Window();

    /*! @brief Sets window width.
     * 
     * This function sets the width of the window.
     * 
     * @param[in] width New width
     */
    void setWidth(int width);

    /*! @brief Sets window height.
     * 
     * This function sets the height of the window.
     * 
     * @param[in] height New height
     */
    void setHeight(int height);

    /*! @brief Sets window size.
     * 
     * This function sets the size (width and height) of the window.
     * 
     * @param[in] width New width
     * @param[in] height New height
     */
    void setSize(int width, int height);

    /*! @brief Sets window title.
     *
     * This function sets the title of the window.
     * 
     * @param[in] title New title
     */
    void setTitle(char* title);

    /*! @brief Shows the window.
     *
     */ 
    void show();

    /*! @brief Hides the window.
     *
     */ 
    void hide();

    void launch(std::vector<Device>& devices);

    void drawFrame();

    /*! @brief Destroys the window.
     *
     */
    void destroy();

    void addShape(Rect& shape);

    /*! @brief Returns whether the specified window should close.
     *
     * @returns 'true' if window should close, 'false' otherwise.
     */ 
    bool shouldClose();

    VkSurfaceKHR getSurface();

protected:


private:

    Device device;

    GLFWwindow* window;
    VkSurfaceKHR surface;

    Swapchain swapchain;
    GraphicsPipeline pipeline;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBufferMemories;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    Dimensions dimensions;
    char* title;

    bool launched;
    bool shown;

    size_t currentFrame;

    std::vector<Rect> shapes;

    void createSwapchain();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createVertexBuffers();
    void createIndexBuffers();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffers();
    void createSyncObjects();
    void destroySwapchain();

    VkShaderModule createShaderModule(const std::vector<char>& code);
};