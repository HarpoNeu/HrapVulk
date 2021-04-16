#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <window.hpp>

/*! @brief Definition of standard application.
 *
 */ 
class Application
{
public:

    /*! @brief Default constructor for 'Application'.
     *
     */
    Application();

    /*! @brief Default destructor for 'Application'.
     *
     * This function frees all components created by the application
     */
    ~Application();

    /*! @brief Initialises application startup.
     *
     * This function initialises the application then transistions into the primary program loop. 
     * Calls virtual function start(). 
     */ 
    void run();

protected:

    /*! @brief Virtual function to be implemented by child class.
     *
     */
    virtual void start(Window& primaryWindow) = 0;

private:

    std::vector<Window> windows;
    std::vector<Device> devices;

};

