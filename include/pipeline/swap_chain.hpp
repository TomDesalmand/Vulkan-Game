#pragma once

// Header files include //
#include "devices/device.hpp"

// Vulkan include //
#include <vulkan/vulkan_core.h>

// STD include //
#include <vector>
#include <memory>

namespace vulkan {

    class SwapChain {
        private:
            Device &_device;
            VkExtent2D _windowExtent;
            VkExtent2D _swapChainExtent;
            VkSwapchainKHR _swapChain;
            VkFormat _swapChainImageFormat;
            VkFormat _swapChainDepthFormat;
            VkRenderPass _renderPass;
            std::shared_ptr<SwapChain> _oldSwapChain;

            std::vector<VkFramebuffer> _swapChainFramebuffers;
            std::vector<VkImage> _depthImages;
            std::vector<VkDeviceMemory> _depthImageMemories;
            std::vector<VkImageView> _depthImageViews;
            std::vector<VkImage> _swapChainImages;
            std::vector<VkImageView> _swapChainImageViews;
            std::vector<VkSemaphore> _imageAvailableSemaphores;
            std::vector<VkSemaphore> _renderFinishedSemaphores;
            std::vector<VkFence> _inFlightFences;
            std::vector<VkFence> _imagesInFlight;
            
            size_t _currentFrame = 0;
            size_t _maxFramesInFlight = 2;

            void init();
            void createSwapChain();
            void createImageViews();
            void createDepthResources();
            void createRenderPass();
            void createFramebuffers();
            void createSyncObjects();

            VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
            VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
            VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

        public:

            SwapChain(Device &deviceRef, VkExtent2D windowExtent, size_t maxFramesInFlight = 2);
            SwapChain(Device &deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous, size_t maxFramesInFlight = 2);
            size_t getMaxFramesInFlight() const { return _maxFramesInFlight; }
            VkFramebuffer getFrameBuffer(int index);
            VkRenderPass getRenderPass();
            VkImageView getImageView(int index);
            size_t getImageCount();
            VkFormat getSwapChainImageFormat();
            VkExtent2D getSwapChainExtent();
            uint32_t getWidth();
            uint32_t getHeight();
            float extentAspectRatio();
            VkFormat findDepthFormat();
            VkResult acquireNextImage(uint32_t *imageIndex);
            VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);
            bool compareSwapFormats(const SwapChain &swapChain) const;
            ~SwapChain();

            // Remove the copy operators to prevent make copies //
            SwapChain(const SwapChain &) = delete;
            SwapChain &operator=(const SwapChain &) = delete;
    };

}