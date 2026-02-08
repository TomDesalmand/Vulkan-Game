#pragma once

// Header files include //
#include "pipeline/pipeline.hpp"
#include "devices/device.hpp"
#include "game/ecs.hpp"

// STD include //
#include <memory>

// Vulkan include //
#include <vulkan/vulkan_core.h>

// GLM include //
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vulkan {

    struct UnifiedPushConstantData {
        alignas(16) glm::mat4 model{1.0f};
        alignas(16) glm::vec3 tintColor{1.0f, 1.0f, 1.0f};
    };

    class UnifiedRenderSystem {
        private:
            Device &_device;
            std::unique_ptr<Pipeline> _pipeline;
            VkPipelineLayout _pipelineLayout{VK_NULL_HANDLE};

            // Descriptor resources (set = 0)
            VkDescriptorSetLayout _descriptorSetLayout{VK_NULL_HANDLE};
            VkDescriptorPool _descriptorPool{VK_NULL_HANDLE};
            VkDescriptorSet _descriptorSet{VK_NULL_HANDLE};

            // Uniform buffer for projection matrix (binding = 0)
            VkBuffer _projUniformBuffer{VK_NULL_HANDLE};
            VkDeviceMemory _projUniformBufferMemory{VK_NULL_HANDLE};

            void createDescriptorResources();
            void createPipelineLayout();
            void createPipeline(VkRenderPass renderPass);

        public:
            UnifiedRenderSystem(Device &device, VkRenderPass renderPass);
            ~UnifiedRenderSystem();

            // Main rendering function
            void renderRegistry(VkCommandBuffer commandBuffer, Registry &registry, const VkExtent2D &extent);

            // Delete copy constructor and assignment
            UnifiedRenderSystem(const UnifiedRenderSystem &) = delete;
            UnifiedRenderSystem &operator=(const UnifiedRenderSystem &) = delete;
    };

} // namespace vulkan