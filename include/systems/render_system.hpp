#pragma once

// Header files include //
#include "pipeline/pipeline.hpp"
#include "devices/device.hpp"
#include "game/ecs.hpp"

// STD include //
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vulkan {

    class RenderSystem {
        private:
            Device &_device;
            std::unique_ptr<Pipeline> _pipeline;
            VkPipelineLayout _pipelineLayout;

            // Descriptor & UBO for global camera/projection (set = 0, binding = 0)
            VkDescriptorSetLayout _descriptorSetLayout{VK_NULL_HANDLE};
            VkDescriptorPool _descriptorPool{VK_NULL_HANDLE};
            VkDescriptorSet _descriptorSet{VK_NULL_HANDLE};

            // Uniform buffer to hold projection matrix
            VkBuffer _projUniformBuffer{VK_NULL_HANDLE};
            VkDeviceMemory _projUniformBufferMemory{VK_NULL_HANDLE};

            void createDescriptorResources();
            void createPipelineLayout();
            void createPipeline(VkRenderPass renderPass);

        public:
            RenderSystem(Device &device, VkRenderPass renderPass);
            void renderRegistry(VkCommandBuffer commandBuffer, Registry &registry, const VkExtent2D &extent);
            ~RenderSystem();

            RenderSystem(const RenderSystem &) = delete;
            RenderSystem &operator=(const RenderSystem &) = delete;
    };

}