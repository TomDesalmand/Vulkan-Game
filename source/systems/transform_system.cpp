// Header files include //
#include "systems/transform_system.hpp"

// STD include //
#include <algorithm>
#include <cmath>

namespace vulkan {

    void TransformSystem::update(Registry &registry, float deltaTime) const {
        if (deltaTime <= 0.0f) {
            return;
        }

        registry.each<TransformComponent, VelocityComponent>([&](Entity, TransformComponent &transform, VelocityComponent &velocity) {
            glm::vec2 totalAccel = velocity.acceleration;
            velocity.velocity += totalAccel * deltaTime;

            if (velocity.damping > 0.0f) {
                const float clamped = std::clamp(velocity.damping, 0.0f, 1.0f);
                velocity.velocity *= (1.0f - clamped * deltaTime);
            }
            if (velocity.maxSpeed > 0.0f) {
                const float speedSq = glm::dot(velocity.velocity, velocity.velocity);
                const float maxSpeedSq = velocity.maxSpeed * velocity.maxSpeed;
                if (speedSq > maxSpeedSq) {
                    const float speed = std::sqrt(speedSq);
                    velocity.velocity *= (velocity.maxSpeed / speed);
                }
            }
            transform.translation += velocity.velocity * deltaTime;
        });
    }

}