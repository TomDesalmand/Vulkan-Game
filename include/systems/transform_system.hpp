#pragma once

// Header files include //
#include "game/ecs.hpp"

namespace vulkan {

    class TransformSystem {
        public:
            void update(Registry &registry, float deltaTime) const;
    };

}