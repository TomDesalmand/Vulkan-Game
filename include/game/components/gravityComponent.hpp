#pragma once

// STD include //
#include <algorithm>

// GLM include //
#include <glm/glm.hpp>

namespace vulkan {
  
    struct GravityComponent {
        glm::vec2 gravity{0.0f, 9.81f};
        bool gravityEnabled{false};
        
        void enableGravity(bool enabled) {
            gravityEnabled = enabled;
        }
        
        void setGravity(const glm::vec2& newGravity) {
            gravity = newGravity;
        }
    };
    
}