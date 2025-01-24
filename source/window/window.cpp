#include "window.hpp"
#include <stdexcept>
#include <iostream>

namespace vulkan {

    Window::Window(int windowWidth, int windowHeight, std::string windowtitle) : _windowWidth(windowWidth), _windowHeight(windowHeight), _windowTitle(windowtitle) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        _window = glfwCreateWindow(_windowWidth, _windowHeight, _windowTitle.c_str(), nullptr, nullptr);
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
            throw std::runtime_error("Failed to create a window surface.");
        }
    }

    void Window::frameBufferResizeCallback(GLFWwindow *window, int width, int height) {
        auto vulkanWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        vulkanWindow->_frameBufferResized = true;
        vulkanWindow->_windowWidth = width;
        vulkanWindow->_windowHeight = height;
    }

    Window::~Window() {
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

}