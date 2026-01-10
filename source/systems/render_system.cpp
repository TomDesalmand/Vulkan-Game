// Header files include //
#include "systems/render_system.hpp"
#include "logger/logging.hpp"

// STD include //
#include <memory>
#include <vector>
#include <stdexcept>
#include <cstring>

// Vulkan include //
#include <vulkan/vulkan_core.h>

// GLM include //
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace vulkan {

    struct SimplePushConstantData {
        alignas(16) glm::mat4 model{1.0f};
        alignas(16) glm::vec3 color{1.f};
    };

    RenderSystem::RenderSystem(Device &device, VkRenderPass renderPass) : _device{device} {
        createDescriptorResources();
        createPipelineLayout();
        createPipeline(renderPass);
    }

    void RenderSystem::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInformation{};
        pipelineLayoutInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        pipelineLayoutInformation.setLayoutCount = 1;
        pipelineLayoutInformation.pSetLayouts = &_descriptorSetLayout;

        pipelineLayoutInformation.pushConstantRangeCount = 1;
        pipelineLayoutInformation.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(_device.getDevice(), &pipelineLayoutInformation, nullptr, &_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout.");
        }
    }

    void RenderSystem::createDescriptorResources() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(_device.getDevice(), &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS) {
            ERROR("Failed to create descriptor set layout.");
            throw std::runtime_error("Failed to create descriptor set layout.");
        }

        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = 1;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 1;

        if (vkCreateDescriptorPool(_device.getDevice(), &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
            ERROR("Failed to create descriptor pool.");
            throw std::runtime_error("Failed to create descriptor pool.");
        }

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &_descriptorSetLayout;

        if (vkAllocateDescriptorSets(_device.getDevice(), &allocInfo, &_descriptorSet) != VK_SUCCESS) {
            ERROR("Failed to allocate descriptor set for projection UBO.");
            throw std::runtime_error("Failed to allocate descriptor set for projection UBO.");
        }

        VkDeviceSize bufferSize = sizeof(glm::mat4);
        _device.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _projUniformBuffer, _projUniformBufferMemory);

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

    void RenderSystem::createPipeline(VkRenderPass renderPass) {
        if (_pipelineLayout == VK_NULL_HANDLE) {
            ERROR("Cannot create pipeline before pipeline layout.");
            throw std::runtime_error("Cannot create pipeline before pipeline layout.");
        }

        PipelineConfigurationInformation pipelineConfiguration{};
        Pipeline::defaultPipelineConfigurationInformation(pipelineConfiguration);
        pipelineConfiguration.renderPass = renderPass;
        pipelineConfiguration.pipelineLayout = _pipelineLayout;
        _pipeline = std::make_unique<Pipeline>(_device, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipelineConfiguration);

    }
    
    void RenderSystem::renderRegistry(VkCommandBuffer commandBuffer, Registry &registry, const VkExtent2D &extent) {
        _pipeline->bind(commandBuffer);

        const float virtualWorldHeight = 5.0f;
        const float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);
        const float worldHalfHeight = virtualWorldHeight * 0.5f;
        const float worldHalfWidth = worldHalfHeight * aspect;

        const float left = -worldHalfWidth;
        const float right = worldHalfWidth;
        const float bottom = -worldHalfHeight;
        const float top = worldHalfHeight;

        glm::mat4 proj = glm::mat4(1.0f);
        proj[0][0] = 2.0f / (right - left);
        proj[1][1] = 2.0f / (top - bottom);
        proj[3][0] = -(right + left) / (right - left);
        proj[3][1] = -(top + bottom) / (top - bottom);

        void *data;
        vkMapMemory(_device.getDevice(), _projUniformBufferMemory, 0, sizeof(glm::mat4), 0, &data);
        memcpy(data, &proj, sizeof(glm::mat4));
        vkUnmapMemory(_device.getDevice(), _projUniformBufferMemory);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSet, 0, nullptr);

        registry.each<ModelComponent, Transform2dComponent, ColorComponent>([&](Entity id, ModelComponent &component, Transform2dComponent &transformComponent, ColorComponent &colorComponet) {
            (void) id;
            transformComponent.rotateObject(0.1f);
            transformComponent.scaleObject(0.25f);

            glm::mat4 model = glm::mat4(1.0f);
            float c = glm::cos(transformComponent.rotation);
            float s = glm::sin(transformComponent.rotation);

            model[0][0] = transformComponent.scale.x * c;
            model[0][1] = transformComponent.scale.x * s;
            model[1][0] = -transformComponent.scale.y * s;
            model[1][1] = transformComponent.scale.y * c;

            model[3][0] = transformComponent.translation.x;
            model[3][1] = transformComponent.translation.y;

            SimplePushConstantData push{};
            push.model = model;
            push.color = colorComponet.color;

            vkCmdPushConstants(commandBuffer, _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

            if (component.model) {
                component.model->bind(commandBuffer);
                component.model->draw(commandBuffer);
            }
        });
    }


    RenderSystem::~RenderSystem() {
        vkDeviceWaitIdle(_device.getDevice());
        if (_projUniformBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(_device.getDevice(), _projUniformBuffer, nullptr);
            _projUniformBuffer = VK_NULL_HANDLE;
        }
        if (_projUniformBufferMemory != VK_NULL_HANDLE) {
            vkFreeMemory(_device.getDevice(), _projUniformBufferMemory, nullptr);
            _projUniformBufferMemory = VK_NULL_HANDLE;
        }
        if (_descriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(_device.getDevice(), _descriptorPool, nullptr);
            _descriptorPool = VK_NULL_HANDLE;
        }
        if (_descriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(_device.getDevice(), _descriptorSetLayout, nullptr);
            _descriptorSetLayout = VK_NULL_HANDLE;
        }
        vkDestroyPipelineLayout(_device.getDevice(), _pipelineLayout, nullptr);
        _pipelineLayout = VK_NULL_HANDLE;
    }

}
