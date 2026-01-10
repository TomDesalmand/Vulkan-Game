#pragma once

// Header files include //
#include "game/ecs.hpp"

// STD include //
#include <cstdint>

namespace vulkan {

    class Registry;
    struct TransformComponent;
    struct VelocityComponent;
    
    class GravitySystem {
        private:
            float _gravityScale{1.0f};
    
        public:
            explicit GravitySystem(float gravityScale = 1.0f) : _gravityScale{gravityScale} {}
            void update(Registry &registry, float deltaTime) const;
            void setGravityScale(float scale);
    };
    
}