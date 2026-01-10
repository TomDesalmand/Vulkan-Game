// Header files include //

#include "window/application.hpp"

#include "systems/render_system.hpp"

#include "systems/gravity_system.hpp"
#include "input/input.hpp"

#include "logger/logging.hpp"


// STD include //
#include <memory>
#include <chrono>

// Vulkan include //
#include <vulkan/vulkan_core.h>

// GLM include //
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/ext/matrix_float2x2.hpp>
#include <glm/ext/vector_float2.hpp>



namespace vulkan {


    Application::Application() {

        loadObjects();

    }



    void Application::run() {

        _renderSystem = std::make_unique<RenderSystem>(_device, _renderer.getSwapChainRenderPass());


        Input input{_window.getGlfwWindow()};

        GravitySystem gravity{};

        
        constexpr double TICKS_PER_SECOND = 128.0;

        constexpr double SECONDS_PER_TICK = 1.0 / TICKS_PER_SECOND;
        
        using clock = std::chrono::high_resolution_clock;
        auto previousTime = clock::now();
        double accumulator = 0.0;
        
        while (!_window.IsClosed()) {
            glfwPollEvents();
            
            auto currentTime = clock::now();
            std::chrono::duration<double> frameDuration = currentTime - previousTime;
            previousTime = currentTime;
            double deltaSeconds = frameDuration.count();
            accumulator += deltaSeconds;
            while (accumulator >= SECONDS_PER_TICK) {
                input.update();                
                gravity.update(_registry, SECONDS_PER_TICK);
                if (input.isKeyPressed(GLFW_KEY_SPACE) == true) {
                    DEBUG("Space key was successfully pressed!");
                    constexpr float JUMP_IMPULSE = -5.0f;
                    _registry.each<Velocity2dComponent>([&](Entity id, Velocity2dComponent &vel) {
                        (void)id;
                        vel.velocity.y = JUMP_IMPULSE;
                        vel.clearAcceleration();
                    });
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
                 accumulator -= SECONDS_PER_TICK;
            }
            if (VkCommandBuffer commandBuffer = _renderer.beginFrame()) {
                _renderer.beginSwapChainRenderPass(commandBuffer);
                _renderSystem->renderRegistry(commandBuffer, _registry, _renderer.getSwapChainExtent());
                _renderer.endSwapChainRenderPass(commandBuffer);
                _renderer.endFrame();
            }
        }   
        _renderSystem.reset();
        vkDeviceWaitIdle(_device.getDevice());
    }

    void Application::loadObjects() {
        std::vector<Model::Vertex> vertecies {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}},
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}
        };

        std::shared_ptr<Model> model = std::make_shared<Model>(_device, vertecies);

        Entity entity = _registry.create_entity();
        _registry.emplace_component<ModelComponent>(entity, ModelComponent{model});
        _registry.emplace_component<ColorComponent>(entity, ColorComponent{{.1f, .8f, .1f}});
        _registry.emplace_component<Transform2dComponent>(entity, Transform2dComponent{{ .2f, 0.0f }, {2.f, .5f}, .25f * glm::two_pi<float>()});
        _registry.emplace_component<Velocity2dComponent>(entity, Velocity2dComponent{{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 9.81f}, true, 0.0f, 10.0f});
    }

    Application::~Application() {
        _renderSystem.reset();
    }

}
