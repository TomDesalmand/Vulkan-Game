#pragma once

// GLFW include and Vulkan setup //
#define GLFW_INCLUDE_VULKAN 
#include <GLFW/glfw3.h>

// STD include //
#include <string>

namespace vulkan {

    class Window {
        private:
            GLFWwindow *_window;
            int _windowWidth;
            int _windowHeight;
            bool _frameBufferResized = false;
            std::string _windowTitle;

            static void frameBufferResizeCallback(GLFWwindow *window, int width, int height);

        public:
            Window(int width, int heigth, std::string windowTitle);
            bool IsClosed();
            bool wasWindowResized();
            void resetWindowResizedFlag();
            VkExtent2D getExtent();
            void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
            ~Window();
            
            // Remove the copy operators to prevent make copies //
            Window(const Window &) = delete;
            Window &operator=(const Window &) = delete;
    };

}