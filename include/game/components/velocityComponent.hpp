#pragma once

// Header files include //
#include "transform2dComponent.hpp"

// STD include //
#include <algorithm>

// GLM include //
#include <glm/glm.hpp>


namespace vulkan {

    struct Velocity2dComponent {
        glm::vec2 velocity{0.0f, 0.0f};
        glm::vec2 acceleration{0.0f, 0.0f};
        glm::vec2 gravity{0.0f, -9.81f};
        bool gravityEnabled{false};
        float damping{0.0f};
        float maxSpeed{10.0f};


        void integrate(float deltaTime) {
            if (deltaTime <= 0.0f) {
                return;
            }
            glm::vec2 totalAccel = acceleration;
            if (gravityEnabled) {
                totalAccel += gravity;
            }
            velocity += totalAccel * deltaTime;
            if (damping > 0.0f) {
                float clamped = std::clamp(damping, 0.0f, 1.0f);
                velocity *= (1.0f - clamped * deltaTime);
            }
            if (maxSpeed > 0.0f) {
                float speedSq = glm::dot(velocity, velocity);
                float maxSpeedSq = maxSpeed * maxSpeed;
                if (speedSq > maxSpeedSq) {
                    float speed = std::sqrt(speedSq);
                    velocity *= (maxSpeed / speed);
                }
            }
        }

        void applyToTransform(Transform2dComponent& transform, float deltaTime) const {
            if (deltaTime <= 0.0f)  {
                return;
            }
            transform.translation += velocity * deltaTime;
        }

        void step(Transform2dComponent& transform, float deltaTime) {
            integrate(deltaTime);
            applyToTransform(transform, deltaTime);
        }
        
        void enableGravity(bool enabled) {
            gravityEnabled = enabled;
        }
        
        void setGravity(const glm::vec2& newGravity) {
            gravity = newGravity;
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