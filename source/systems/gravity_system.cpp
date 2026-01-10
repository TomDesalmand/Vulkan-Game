// Header files include //
#include "systems/gravity_system.hpp"

namespace vulkan {

    void GravitySystem::update(Registry &registry, float deltaTime) const {
        if (deltaTime <= 0.0f) {
            return;
        }
        
        registry.each<GravityComponent, VelocityComponent>([&](Entity, GravityComponent &gravity, VelocityComponent &velocity) {
            if (gravity.gravityEnabled == true) {
                velocity.addVelocityY(gravity.gravity.y * deltaTime);
            }
        });
    }
    
    void GravitySystem:: setGravityScale(float scale) {
        _gravityScale = scale;
    }

}