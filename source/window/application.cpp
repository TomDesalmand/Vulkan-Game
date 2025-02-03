#include "window/application.hpp"

// GLM include //
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <stdexcept>
#include <array>
#include <iostream>
#include <cassert>

namespace vulkan {

    struct SimplePushConstantData {
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    Application::Application() {
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    }

    void Application::run() {
        while (!_window.IsClosed()) {
            glfwPollEvents();
            drawFrame();
        }
        vkDeviceWaitIdle(_device.getDevice());
    }

    void Application::loadModels() {
        std::vector<Model::Vertex> vertecies {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };

        _model = std::make_unique<Model>(_device, vertecies);
    }

    void Application::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInformation{};
        pipelineLayoutInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInformation.setLayoutCount = 0;
        pipelineLayoutInformation.pSetLayouts = nullptr;
        pipelineLayoutInformation.pushConstantRangeCount = 1;
        pipelineLayoutInformation.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(_device.getDevice(), &pipelineLayoutInformation, nullptr, &_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout.");
        }
    }

    void Application::createPipeline() {
        assert(_swapChain != nullptr && "Cannot create pipeline before swap-chain.");
        assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout.");

        PipelineConfigurationInformation pipelineConfiguration{};
        Pipeline::defaultPipelineConfigurationInformation(pipelineConfiguration);
        pipelineConfiguration.renderPass = _swapChain->getRenderPass();
        pipelineConfiguration.pipelineLayout = _pipelineLayout;
        _pipeline = std::make_unique<Pipeline>(_device, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipelineConfiguration);

    }

    void Application::recreateSwapChain() {
        VkExtent2D extent = _window.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = _window.getExtent();
            glfwPollEvents();
        }

        vkDeviceWaitIdle(_device.getDevice());

        if (_swapChain == nullptr) {
            _swapChain = std::make_unique<SwapChain>(_device, extent);
        } else {
            _swapChain = std::make_unique<SwapChain>(_device, extent, std::move(_swapChain));
            if (_swapChain->getImageCount() != _commandBuffers.size()) {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }
        createPipeline();
    }

    void Application::createCommandBuffers() {
        _commandBuffers.resize(_swapChain->getImageCount());

        VkCommandBufferAllocateInfo allocatedInformation{};
        allocatedInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocatedInformation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocatedInformation.commandPool = _device.getCommandPool();
        allocatedInformation.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());

        if (vkAllocateCommandBuffers(_device.getDevice(), &allocatedInformation, _commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers.");
        }
    }

    void Application::freeCommandBuffers() {
        vkFreeCommandBuffers(_device.getDevice(), _device.getCommandPool(), static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());
        _commandBuffers.clear();
    }

    void Application::recordCommandBuffer(int imageIndex) {
        static int frame = 0;
        frame = (frame + 1) % 1000;

        VkCommandBufferBeginInfo beginInformation{};
        beginInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(_commandBuffers[imageIndex], &beginInformation) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer.");
        }

        VkRenderPassBeginInfo renderPassInformation{};
        renderPassInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInformation.renderPass = _swapChain->getRenderPass();
        renderPassInformation.framebuffer = _swapChain->getFrameBuffer(imageIndex);
        renderPassInformation.renderArea.offset = {0, 0};
        renderPassInformation.renderArea.extent = _swapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.01f, 0.01f, 0.01f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInformation.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInformation.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(_commandBuffers[imageIndex], &renderPassInformation, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(_swapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(_swapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, _swapChain->getSwapChainExtent()};
        vkCmdSetViewport(_commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(_commandBuffers[imageIndex], 0, 1, &scissor);

        _pipeline->bind(_commandBuffers[imageIndex]);
        _model->bind(_commandBuffers[imageIndex]);

        for (int i = 0; i < 4; i++) {
            SimplePushConstantData push{};
            push.offset = {0.5f + frame * 0.005f, -0.5f * i * 0.25f};
            push.color = {0.0f, 0.0f, 0.2f + 0.2f * i};
            vkCmdPushConstants(_commandBuffers[imageIndex], _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
            _model->draw(_commandBuffers[imageIndex]);
        }

        vkCmdEndRenderPass(_commandBuffers[imageIndex]);

        if (vkEndCommandBuffer(_commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer.");
        }
    }

    void Application::drawFrame() {
        uint32_t imageIndex;
        VkResult result = _swapChain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire next swap-chain image.");
        }

        recordCommandBuffer(imageIndex);
        result = _swapChain->submitCommandBuffers(&_commandBuffers[imageIndex], &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _window.wasWindowResized()) {
            _window.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap-chain image.");
        }
    }

    Application::~Application() {
        vkDestroyPipelineLayout(_device.getDevice(), _pipelineLayout, nullptr);
    }

}
