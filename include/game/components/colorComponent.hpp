#pragma once

// GLM include //
#include <glm/glm.hpp>
#include <glm/ext/matrix_float2x2.hpp>
#include <glm/gtc/constants.hpp>

namespace vulkan {

    struct ColorComponent {
        glm::vec3 color{1.f, 1.f, 1.f};

        void setFrom255(uint8_t r, uint8_t g, uint8_t b) {
            color = glm::vec3(
                glm::clamp(r / 255.0f, 0.0f, 1.0f),
                glm::clamp(g / 255.0f, 0.0f, 1.0f),
                glm::clamp(b / 255.0f, 0.0f, 1.0f)
            );
        }
    };
    
}