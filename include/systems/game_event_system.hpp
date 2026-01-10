#pragma once

// Header files include //
#include "game/ecs.hpp"
#include "input/input.hpp"

namespace vulkan {
    
    class GameEventSystem {
        private:
            void jump(Registry &registry) const;
            
        public:
            void update(Registry &registry, Input &input) const;
    };

}