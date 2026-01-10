// Header files include //
#include "systems/game_event_system.hpp"
#include "logger/logging.hpp"

namespace vulkan {

    void GameEventSystem::update(Registry &registry, Input &input) const {
        if (input.isKeyPressed(GLFW_KEY_SPACE) == true) {
            jump(registry);
        }
        if (input.isMousePressed(GLFW_MOUSE_BUTTON_LEFT) == true) {
            DEBUG("Left click was successfully pressed!");
            auto pos = input.getMousePosition();
            DEBUG("Coordinates: X:", pos.first, ", Y:", pos.second);
        }
        if (input.isMousePressed(GLFW_MOUSE_BUTTON_RIGHT) == true) {
            DEBUG("Right click was successfully pressed!");
            auto pos = input.getMousePosition();
            DEBUG("Coordinates: X:", pos.first, ", Y:", pos.second);
        }
    }
    
    void GameEventSystem::jump(Registry &registry) const {
        registry.each<VelocityComponent>([&](Entity id, VelocityComponent &velocityComponent) {
            (void)id;
            // Jump velocity power //
            velocityComponent.velocity.y = -5.0f;
            velocityComponent.clearAcceleration();
        });
    }

}