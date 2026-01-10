#pragma once

// STD include //
#include <cmath>

// GLM include //
#include <glm/glm.hpp>
#include <glm/ext/matrix_float2x2.hpp>
#include <glm/gtc/constants.hpp>

namespace vulkan {
    
    struct Transform2dComponent {
        glm::vec2 translation{0.0f, 0.0f};
        glm::vec2 scale{1.0f, 1.0f};
        float rotation{0.0f};
        
        void rotateObject(float degreeAngle) {
            const float radians = degreeAngle * glm::pi<float>() / 180.0f;
            rotation += radians;
            rotation = std::fmod(rotation, glm::two_pi<float>());
            if (rotation < 0.0f) {
                rotation += glm::two_pi<float>();
            }
        }
        
        void scaleObject(float newScale) {
            scale = glm::vec2{newScale, newScale};
        }
        
        void scaleObjectX(float newScaleX) {
            scale.x = newScaleX;
        }
        void scaleObjectY(float newScaleY) {
            scale.y = newScaleY;
        }
        
        void moveObjectX(float x) {
            translation.x += x * scale.x / 100.f;
        }
        
        void moveObjectY(float y) {
            translation.y += y * scale.y / 100.f;
        }
    
        glm::mat2 applyTransform() const {
            const float s = glm::sin(rotation);
            const float c = glm::cos(rotation);
            glm::mat2 rotationMatrix{{c, s}, {-s, c}};
            glm::mat2 scaleMatrix{{scale.x, 0.0f}, {0.0f, scale.y}};
            return rotationMatrix * scaleMatrix;
        }
    };
    
}