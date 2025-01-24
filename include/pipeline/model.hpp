#pragma once

#include "../devices/device.hpp"

// GLM include //
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// STD include //
#include <vector>

namespace vulkan {

    class Model {
        private:
            Device &_device;
            VkBuffer _vertexBuffer;
            VkDeviceMemory _vertexBufferMemory;
            uint32_t _vertexCount;


        public:

            struct Vertex {
                glm::vec2 position;
                glm::vec3 color;
                static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
                static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            };

            Model(Device &device, const std::vector<Vertex> &vertices);
            void createVertexBuffers(const std::vector<Vertex> &vertices);
            void bind(VkCommandBuffer commandBuffer);
            void draw(VkCommandBuffer commandBuffer);
            ~Model();

            // Remove the copy operators to prevent make copies //
            Model(const Model &) = delete;
            Model &operator=(const Model &) = delete;
    };

}