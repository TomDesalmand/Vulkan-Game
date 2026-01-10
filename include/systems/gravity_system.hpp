#pragma once

// STD include //
#include <cstdint>

namespace vulkan {

    class Registry;
    struct Transform2dComponent;
    struct Velocity2dComponent;
    
    class GravitySystem {
        private:
            float _gravityScale{1.0f};
    
        public:
            explicit GravitySystem(float gravityScale = 1.0f) noexcept : _gravityScale{gravityScale} {}
    
            void update(Registry &registry, double dt) const;
            
            void setGravityScale(float scale) noexcept {
                _gravityScale = scale;
            }
            float getGravityScale() const noexcept {
                return _gravityScale;
            }
    };
    
}