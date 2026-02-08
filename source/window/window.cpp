// Header files include //
#include "window/window.hpp"
#include "logger/logging.hpp"

// STD include //
#include <stdexcept>

namespace vulkan {

    Window::Window(int windowWidth, int windowHeight, std::string windowtitle) : _windowWidth(windowWidth), _windowHeight(windowHeight), _windowTitle(windowtitle) {
        if (glfwInit() != GLFW_TRUE) {
            ERROR("Failed to initialize GLFW.");
            throw std::runtime_error("Failed to initialize GLFW.");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        _window = glfwCreateWindow(_windowWidth, _windowHeight, _windowTitle.c_str(), nullptr, nullptr);
        if (_window == nullptr) {
            ERROR("Failed to create GLFW window.");
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window.");
        }
        glfwSetWindowUserPointer(_window, this);
        glfwSetFramebufferSizeCallback(_window, frameBufferResizeCallback);
    }

    bool Window::IsClosed() {
        return glfwWindowShouldClose(_window);
    }

    bool Window::wasWindowResized() {
        return _frameBufferResized;
    }

    void Window::resetWindowResizedFlag() {
        _frameBufferResized = false;
    }

    VkExtent2D Window::getExtent() {
        return {static_cast<uint32_t>(_windowWidth), static_cast<uint32_t>(_windowHeight)};
    }

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if (glfwCreateWindowSurface(instance, _window, nullptr, surface) != VK_SUCCESS) {
            ERROR("Failed to create a window surface.");
            throw std::runtime_error("Failed to create a window surface.");
        }
    }

    void Window::frameBufferResizeCallback(GLFWwindow *window, int width, int height) {
        auto vulkanWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        vulkanWindow->_frameBufferResized = true;
        vulkanWindow->_windowWidth = width;
        vulkanWindow->_windowHeight = height;
    }

    GLFWwindow* Window::getGlfwWindow() {
        return _window;
    }

    Window::~Window() {
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

}
