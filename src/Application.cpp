#include <hvulk.hpp>

#include <debug.hpp>
#include <device.hpp>

#include <stdexcept>
#include <vector>

VkInstance instance;
VkDebugUtilsMessengerEXT debugMessenger;

/*! @brief Checks that requested validation layers are present on the system.
 *
 * This function retrieves all available validation layers and compares them to the requested layers.
 * 
 * @param[in] debugLayers Requested debug layers
 * 
 * @return 'true' if all layers can be found, otherwise 'false'.
 */
bool checkValidationSupport(const std::vector<const char*>& debugLayers)
{
    //retrieve supported layers
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> supportedLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, supportedLayers.data());

    //enumerate over requested layers
    for (const char* layer : debugLayers)
    {
        bool layerFound = false;

        //compare requested layer to supported layers
        for (const auto& supportedLayer : supportedLayers)
        {
            if (strcmp(layer, supportedLayer.layerName) == 0)
            {
                //requested layer is supported
                layerFound = true;
                break;
            }
        }

        //requested layer is not supported
        if (!layerFound)
        {
            return false;
        }
    }

    //requested layers are supported
    return true;
}

/*! @brief Gets required instance extensions for GLFW and Debug Utils.
 * 
 * @return Vector containing required extensions.
 */
std::vector<const char*> getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

/*! @brief Creates instance of Vulkan library.
 *
 * This function creates an instance of the Vulkan library linked with a debug messenger.
 * 
 */
void createInstance()
{
    //define application info
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.apiVersion = VK_API_VERSION_1_1;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = "Application";
    applicationInfo.pEngineName = "No Engine";
    applicationInfo.pNext = nullptr; 

    //query debug messenger create info
    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
    queryDebugCreateInfo(&messengerCreateInfo);

    //define instance create info
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    //retrieve required extensions
    auto instanceExtensions = getRequiredExtensions();

    //requested debug layers
    const std::vector<const char*> debugLayers {
        "VK_LAYER_KHRONOS_validation"
    };

    //check requested debug layers are supported 
    if (!checkValidationSupport(debugLayers))
    {
        throw std::runtime_error("Error! Requested validation layer not supported!");
    }

    //request extensions / layers
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(debugLayers.size());
    instanceCreateInfo.ppEnabledLayerNames = debugLayers.data();

    //next create debug messenger
    instanceCreateInfo.pNext = &messengerCreateInfo;

    //create instance
    if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS)
    {
        //throw runtime_error if fails
        throw std::runtime_error("Error! Failed to create instance!");
    }

    //create debug messenger
    if (createDebugUtilsMessengerEXT(instance, &messengerCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        //throw runtime_error if fails
        throw std::runtime_error("Error! Failed to create debug messenger!");
    }
}

/*! @brief Gets the vulkan instance created by the application.
 *
 * @return Vulkan instance
 */
VkInstance getInstance()
{
    return instance;
}

/*! @brief Implementation of default constructor for 'Application' class.
 *
 */
Application::Application()
{
    //initialise glfw
    if (!glfwInit())
    {
        throw std::runtime_error("Error! Failed to initialise GLFW!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    Window primaryWindow;
    windows.push_back(primaryWindow);
}

/*! @brief Implementation of default destructor for 'Application' class.
 *
 * This function destroys debugMessenger, instance.
 */
Application::~Application()
{
    for (Window window : windows)
    {
        window.destroy();
    }

    for (Device device : devices)
    {
        device.destroy();
    }

    //destroy debugMessenger
    destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    //destroy instance
    vkDestroyInstance(instance, nullptr);

}

/*! @brief Implementation of Application::run().
 *
 * This function calls start() followed by createInstance().
 */
void Application::run()
{
    createInstance();

    start(windows[0]);

    windows[0].launch(devices);

    while(!windows[0].shouldClose())
    {
        glfwPollEvents();

        for (Window window : windows)
        {
            window.drawFrame();
        }
    }
}
