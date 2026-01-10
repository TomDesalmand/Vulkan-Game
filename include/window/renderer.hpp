#pragma once

// Header files include //
#include "window.hpp"
#include "pipeline/swap_chain.hpp"
#include "devices/device.hpp"

// STD include //
#include <memory>
#include <vector>

// Vulkan include //
#include <vulkan/vulkan_core.h>

namespace vulkan {

    class Renderer {
        private:
            uint32_t _currentImageIndex{0};
            int _currentFrameIndex{0};
            bool _frameStarted{false};
            Window &_window;
            Device &_device;
            std::unique_ptr<SwapChain> _swapChain;
            std::vector<VkCommandBuffer> _commandBuffers;

            void createCommandBuffers();
            void freeCommandBuffers();
            void recreateSwapChain();

        public:
            Renderer(Window &window, Device &device);
            VkCommandBuffer beginFrame();
            void endFrame();
            void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
            void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
            bool getFrameProgress();
            int getFrameIndex();
            VkCommandBuffer getCurrentCommandBuffer();
            VkRenderPass getSwapChainRenderPass() const;
            VkExtent2D getSwapChainExtent() const;
            ~Renderer();

            // Remove the copy operators to prevent make copies //
            Renderer(const Renderer &) = delete;
            Renderer &operator=(const Renderer &) = delete;
    };

}