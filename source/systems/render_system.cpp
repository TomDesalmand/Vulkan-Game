// Header files include //
#include "systems/render_system.hpp"
#include "game/components/modelComponent.hpp"
#include "game/components/transformComponent.hpp"
#include "game/components/colorComponent.hpp"

// STD include //
#include <memory>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <cmath>
#include <cassert>

// Vulkan include //
#include <vulkan/vulkan_core.h>

// GLM include //
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace vulkan {

    UnifiedRenderSystem::UnifiedRenderSystem(Device &device, VkRenderPass renderPass) : _device{device} {
        createDescriptorResources();
        createPipelineLayout();
        createPipeline(renderPass);
    }

    void UnifiedRenderSystem::createDescriptorResources() {
        // Simple descriptor set layout with just projection matrix uniform
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &binding;

        if (vkCreateDescriptorSetLayout(_device.getDevice(), &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }

        // Create descriptor pool
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = 1;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 1;

        if (vkCreateDescriptorPool(_device.getDevice(), &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool!");
        }

        // Allocate descriptor set
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &_descriptorSetLayout;

        if (vkAllocateDescriptorSets(_device.getDevice(), &allocInfo, &_descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }

        // Create projection matrix uniform buffer
        VkDeviceSize bufferSize = sizeof(glm::mat4);
        _device.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            _projUniformBuffer,
            _projUniformBufferMemory
        );

        // Update descriptor set with uniform buffer
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = _projUniformBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(glm::mat4);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = _descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(_device.getDevice(), 1, &descriptorWrite, 0, nullptr);
    }

    void UnifiedRenderSystem::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(UnifiedPushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(_device.getDevice(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }
    }

    void UnifiedRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(_pipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline before pipeline layout");

        PipelineConfigurationInformation pipelineConfig{};
        Pipeline::defaultPipelineConfigurationInformation(pipelineConfig);

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = _pipelineLayout;

        // Configure vertex input for Model::Vertex
        pipelineConfig.vertexBindingDescriptions = Model::Vertex::getBindingDescriptions();
        pipelineConfig.vertexAttributeDescriptions = Model::Vertex::getAttributeDescriptions();

        _pipeline = std::make_unique<Pipeline>(
            _device,
            "shaders/render_shader.vert.spv",
            "shaders/render_shader.frag.spv",
            pipelineConfig
        );
    }

    void UnifiedRenderSystem::renderRegistry(VkCommandBuffer commandBuffer, Registry &registry, const VkExtent2D &extent) {
        if (!_pipeline) return;

        _pipeline->bind(commandBuffer);

        // Calculate orthographic projection matrix
        const float virtualWorldHeight = 5.0f;
        const float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);
        const float worldHalfHeight = virtualWorldHeight * 0.5f;
        const float worldHalfWidth = worldHalfHeight * aspect;

        glm::mat4 proj = glm::mat4(1.0f);
        proj[0][0] = 1.0f / worldHalfWidth;
        proj[1][1] = 1.0f / worldHalfHeight;
        proj[3][0] = 0.0f;
        proj[3][1] = 0.0f;

        // Update projection matrix in uniform buffer
        void *data = nullptr;
        vkMapMemory(_device.getDevice(), _projUniformBufferMemory, 0, sizeof(glm::mat4), 0, &data);
        std::memcpy(data, &proj, sizeof(glm::mat4));
        vkUnmapMemory(_device.getDevice(), _projUniformBufferMemory);

        // Bind descriptor set
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 
                               0, 1, &_descriptorSet, 0, nullptr);

        // Render geometry (triangles, rectangles, etc.)
        registry.each<ModelComponent, TransformComponent, ColorComponent>([&](Entity /*id*/, ModelComponent &modelComp, TransformComponent &transform, ColorComponent &colorComp) {
            if (!modelComp.model) return;

            // Apply demo rotation
            transform.rotateObject(0.1f);
            
            // Build transformation matrix
            float c = glm::cos(transform.rotation);
            float s = glm::sin(transform.rotation);

            glm::mat4 model = glm::mat4(1.0f);
            model[0][0] = transform.scale.x * c;
            model[0][1] = transform.scale.x * s;
            model[1][0] = -transform.scale.y * s;
            model[1][1] = transform.scale.y * c;
            model[3][0] = transform.translation.x;
            model[3][1] = transform.translation.y;

            // Push constants for solid color mode
            UnifiedPushConstantData push{};
            push.model = model;
            push.tintColor = colorComp.color;

            vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                              0, sizeof(UnifiedPushConstantData), &push);

            modelComp.model->bind(commandBuffer);
            modelComp.model->draw(commandBuffer);
        });
    }

    UnifiedRenderSystem::~UnifiedRenderSystem() {
        // Wait for all operations to complete before cleanup
        vkDeviceWaitIdle(_device.getDevice());

        // Clean up Vulkan resources in reverse order of creation
        if (_projUniformBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(_device.getDevice(), _projUniformBuffer, nullptr);
        }
        if (_projUniformBufferMemory != VK_NULL_HANDLE) {
            vkFreeMemory(_device.getDevice(), _projUniformBufferMemory, nullptr);
        }
        if (_descriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(_device.getDevice(), _descriptorPool, nullptr);
        }
        if (_descriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(_device.getDevice(), _descriptorSetLayout, nullptr);
        }
        if (_pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(_device.getDevice(), _pipelineLayout, nullptr);
        }
    }

} // namespace vulkan