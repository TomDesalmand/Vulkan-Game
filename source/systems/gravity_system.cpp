// Header files include //
#include "systems/gravity_system.hpp"
#include "game/ecs.hpp"
#include "game/components/transform2dComponent.hpp"
#include "game/components/velocityComponent.hpp"

// STD include //
#include <cmath>

namespace vulkan {

    void GravitySystem::update(Registry &registry, double dt) const {
        const float deltaTime = static_cast<float>(dt);
        if (deltaTime <= 0.0f) {
            return;
        }

        registry.each<Transform2dComponent, Velocity2dComponent>([&](Entity, Transform2dComponent &transform, Velocity2dComponent &velocity) {
            if (velocity.gravityEnabled) {
                const glm::vec2 originalGravity = velocity.gravity;
                velocity.gravity = originalGravity * _gravityScale;
                velocity.step(transform, deltaTime);
                velocity.gravity = originalGravity;
            } else {
                velocity.step(transform, deltaTime);
            }
        });
    }

}