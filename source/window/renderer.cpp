// Header files include //
#include "window/renderer.hpp"
#include "logger/logging.hpp"

// STD include //
#include <memory>
#include <stdexcept>
#include <array>

// Vulkan include //
#include <vulkan/vulkan_core.h>

namespace vulkan {
    
    Renderer::Renderer(Window &window, Device &device) : _window{window}, _device{device} {
        recreateSwapChain();
        createCommandBuffers();
    }
    
    bool Renderer::getFrameProgress() {
        return _frameStarted;
    }
    
    VkCommandBuffer Renderer::getCurrentCommandBuffer() {
        if (!_frameStarted) {
            ERROR("Failed retrieving command buffer since frame is not in progress.");
            throw std::runtime_error("Failed retrieving command buffer since frame is not in progress.");
        }
        return _commandBuffers[_currentFrameIndex];
    }
    
    VkRenderPass Renderer::getSwapChainRenderPass() const {
        return _swapChain->getRenderPass();
    }

    VkExtent2D Renderer::getSwapChainExtent() const {
        if (_swapChain == nullptr) {
            ERROR("Swap chain not created when querying extent.");
            throw std::runtime_error("Swap chain not created when querying extent.");
        }
        return _swapChain->getSwapChainExtent();
    }
    
    int Renderer::getFrameIndex() {
        if (!_frameStarted) {
            ERROR("Can't run get frame index while a frame is not in progress.");
            throw std::runtime_error("Can't run get frame index while a frame is not in progress.");
        }
        return _currentFrameIndex;
    }

    void Renderer::recreateSwapChain() {
        VkExtent2D extent = _window.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = _window.getExtent();
            glfwPollEvents();
        }

        vkDeviceWaitIdle(_device.getDevice());

        if (_swapChain == nullptr) {
            _swapChain = std::make_unique<SwapChain>(_device, extent);
        } else {
            std::shared_ptr<SwapChain> oldSwapChain = std::move(_swapChain);
            _swapChain = std::make_unique<SwapChain>(_device, extent, oldSwapChain);
            if (!oldSwapChain->compareSwapFormats(*_swapChain.get())) {
                throw std::runtime_error("Swap chain image/depth format has changed.");
            }
            if (_swapChain->getImageCount() != _commandBuffers.size()) {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }
    }

    void Renderer::createCommandBuffers() {
        _commandBuffers.resize(2); // MAX_FRAMES_IN_FLIGHT
        VkCommandBufferAllocateInfo allocatedInformation{};
        allocatedInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocatedInformation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocatedInformation.commandPool = _device.getCommandPool();
        allocatedInformation.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

        if (vkAllocateCommandBuffers(_device.getDevice(), &allocatedInformation, _commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers.");
        }
    }

    void Renderer::freeCommandBuffers() {
        vkFreeCommandBuffers(_device.getDevice(), _device.getCommandPool(), static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());
        _commandBuffers.clear();
    }

    VkCommandBuffer Renderer::beginFrame() {
        if (_frameStarted) {
            ERROR("Can't run begin frame while a frame is in progress.");
            throw std::runtime_error("Can't run begin frame while a frame is in progress.");
        }
        VkResult result = _swapChain->acquireNextImage(&_currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire next swap-chain image.");
        }
        _frameStarted = true;
        VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInformation{};
        beginInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInformation) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer.");
        }
        return commandBuffer;
    }
    
    void Renderer::endFrame() {
        if (!_frameStarted) {
            ERROR("Can't run end frame while a frame is not in progress.");
            throw std::runtime_error("Can't run end frame while a frame is not in progress.");
        }
        VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer.");
        }
        VkResult result = _swapChain->submitCommandBuffers(&commandBuffer, &_currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _window.wasWindowResized()) {
            _window.resetWindowResizedFlag();
            recreateSwapChain();
            _frameStarted = false;
            _currentFrameIndex = (_currentFrameIndex + 1) % 2; // MAX_FRAMES_IN_FLIGHT
            return;
        }
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap-chain image.");
        }
        _frameStarted = false;
        _currentFrameIndex = (_currentFrameIndex + 1) % 2; // MAX_FRAMES_IN_FLIGHT
    }
    
    void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        if (!_frameStarted) {
            ERROR("Can't run begin swapchain render pass while a frame is not in progress.");
            throw std::runtime_error("Can't run begin swapchain render pass while a frame is not in progress.");
        }
        if (commandBuffer != getCurrentCommandBuffer()) {
            ERROR("Can't run begin swapchain render pass on a different buffer than the current frame");
            throw std::runtime_error("Can't run begin swapchain render pass on a different buffer than the current frame");
        }
        VkRenderPassBeginInfo renderPassInformation{};
        renderPassInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInformation.renderPass = _swapChain->getRenderPass();
        renderPassInformation.framebuffer = _swapChain->getFrameBuffer(_currentImageIndex);
        renderPassInformation.renderArea.offset = {0, 0};
        renderPassInformation.renderArea.extent = _swapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.01f, 0.01f, 0.01f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInformation.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInformation.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInformation, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(_swapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(_swapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, _swapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }
    
    void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        if (!_frameStarted) {
            ERROR("Can't run end swapchain render pass while a frame is not in progress.");
            throw std::runtime_error("Can't run end swapchain render pass while a frame is not in progress.");
        }
        if (commandBuffer != getCurrentCommandBuffer()) {
            ERROR("Can't run end swapchain render pass on a different buffer than the current frame");
            throw std::runtime_error("Can't run end swapchain render pass on a different buffer than the current frame");
        }
        vkCmdEndRenderPass(commandBuffer);
    }

    Renderer::~Renderer() {
        freeCommandBuffers();
    }
} 