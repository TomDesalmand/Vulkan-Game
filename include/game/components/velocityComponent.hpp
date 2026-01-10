#pragma once

// Header files include //
#include "transformComponent.hpp"

// STD include //
#include <algorithm>

// GLM include //
#include <glm/ext/vector_float2.hpp>
#include <glm/glm.hpp>

namespace vulkan {

    struct VelocityComponent {
        glm::vec2 velocity{0.0f, 0.0f};
        glm::vec2 acceleration{0.0f, 0.0f};
        float damping{0.0f};
        float maxSpeed{10.0f};
        
        void setVeclocity(glm::vec2 newVelocity) {
            velocity = newVelocity;
        }
        
        void addVelocityX(float newVelocityX) {
            velocity.x += newVelocityX;
        }
        
        void addVelocityY(float newVelocityY) {
            velocity.y += newVelocityY;
        }
        
        void setDamping(float newDamping) {
            damping = newDamping;
        }
        
        void setMaxSpeed(float newMaxSpeed) {
            maxSpeed = newMaxSpeed;
        }

        void clearAcceleration() {
            acceleration = glm::vec2{0.0f, 0.0f};
        }
        
        void stop() {
            velocity = glm::vec2{0.0f, 0.0f};
        }
    };

}