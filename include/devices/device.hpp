#pragma once

// Header files include //
#include "window/window.hpp"

// STD include //
#include <vector>

// Vulkan include //
#include <vulkan/vulkan_core.h>

#ifdef __APPLE__
#ifndef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
#define VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME "VK_KHR_portability_enumeration"
#endif
#ifndef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
#define VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME "VK_KHR_portability_subset"
#endif
#endif

namespace vulkan {

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices {
        uint32_t graphicsFamily;
        uint32_t presentFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    class Device {
        private:
            VkInstance _instance;
            VkDebugUtilsMessengerEXT _debugMessenger;
            VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
            Window &_window;
            VkCommandPool _commandPool;

            VkDevice _device;
            VkSurfaceKHR _surface;
            VkQueue _graphicsQueue;
            VkQueue _presentQueue;

            const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
#ifdef __APPLE__
            const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME};
#else
            const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#endif


            void createInstance();
            void setupDebugMessenger();
            void createSurface();
            void pickPhysicalDevice();
            void createLogicalDevice();
            void createCommandPool();
            bool isDeviceSuitable(VkPhysicalDevice device);
            std::vector<const char *> getRequiredExtensions();
            bool checkValidationLayerSupport();
            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
            void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
            void hasGflwRequiredInstanceExtensions();
            bool checkDeviceExtensionSupport(VkPhysicalDevice device);
            SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

        public:
            // Enable this when Debug mode and disable for release //
            const bool enableValidationLayers = true;
            // =================================================== //

            VkPhysicalDeviceProperties _properties;

            Device(Window &window);
            VkCommandPool getCommandPool();
            VkDevice getDevice();
            VkSurfaceKHR getSurface();
            VkQueue getGraphicsQueue();
            VkQueue getPresentQueue();
            SwapChainSupportDetails getSwapChainSupport();
            QueueFamilyIndices findPhysicalQueueFamilies();
            uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
            VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
            void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
            VkCommandBuffer beginSingleTimeCommands();
            void endSingleTimeCommands(VkCommandBuffer commandBuffer);
            void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
            void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
            void createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
            ~Device();

            // Remove the copy operators to prevent make copies //
            Device(const Device &) = delete;
            Device &operator=(const Device &) = delete;
            Device(Device &&) = delete;
            Device &operator=(Device &&) = delete;
    };

}
