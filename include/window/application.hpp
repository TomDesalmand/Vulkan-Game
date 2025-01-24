#pragma once

// Code include //
#include "window.hpp"
#include "../pipeline/pipeline.hpp"
#include "../pipeline/swap_chain.hpp"
#include "../pipeline/model.hpp"
#include "../devices/device.hpp"

// STD include //
#include <memory>
#include <vector>

namespace vulkan {

    class Application {
        private:
            static constexpr int WIDTH = 1920;
            static constexpr int HEIGHT = 1080;
            Window _window{WIDTH, HEIGHT, "Vulkan Application"};
            Device _device{_window};
            std::unique_ptr<SwapChain> _swapChain;
            std::unique_ptr<Pipeline> _pipeline;
            VkPipelineLayout _pipelineLayout;
            std::vector<VkCommandBuffer> _commandBuffers;
            std::unique_ptr<Model> _model;
            std::vector<VkFence> _commandBufferFences;

            void loadModels();
            void createPipelineLayout();
            void createPipeline();
            void createCommandBuffers();
            void freeCommandBuffers();
            void drawFrame();
            void recreateSwapChain();
            void recordCommandBuffer(int imageIndex);

        public:
            Application();
            void run();
            ~Application();

            // Remove the copy operators to prevent make copies //
            Application(const Application &) = delete;
            Application &operator=(const Application &) = delete;
    };

}