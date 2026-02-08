// Header files include //
#include "window/application.hpp"
#include "input/input.hpp"
#include "systems/transform_system.hpp"
#include "systems/gravity_system.hpp"
#include "systems/game_event_system.hpp"

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
        // Tick Clock //
        time previousTime = clock::now();
        double accumulator = 0.0;
        
        // Systems //
        Input input{_window.getGlfwWindow()};
        _unifiedRenderSystem = std::make_unique<UnifiedRenderSystem>(_device, _renderer.getSwapChainRenderPass());
        GravitySystem gravity{};
        TransformSystem transform{};
        GameEventSystem gameEvent{};
        

        
        while (!_window.IsClosed()) {
            glfwPollEvents();
            updateClock(previousTime, accumulator);
            while (accumulator >= SECONDS_PER_TICK) {
                input.update();                
                gravity.update(_registry, SECONDS_PER_TICK);
                transform.update(_registry, SECONDS_PER_TICK);
                gameEvent.update(_registry, input);
                accumulator -= SECONDS_PER_TICK;
            }
            if (VkCommandBuffer commandBuffer = _renderer.beginFrame()) {
                _renderer.beginSwapChainRenderPass(commandBuffer);
                _unifiedRenderSystem->renderRegistry(commandBuffer, _registry, _renderer.getSwapChainExtent());
                _renderer.endSwapChainRenderPass(commandBuffer);
                _renderer.endFrame();
            }
        }   
        _unifiedRenderSystem.reset();
        vkDeviceWaitIdle(_device.getDevice());
    }
    
    void Application::updateClock(time &previousTime, double &accumulator) {
        time currentTime = clock::now();
        std::chrono::duration<double> frameDuration = currentTime - previousTime;
        previousTime = currentTime;
        double deltaSeconds = frameDuration.count();
        accumulator += deltaSeconds;
    }

    void Application::loadObjects() {
        std::vector<Model::Vertex> vertecies {
            {{-0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
            {{0.5f, -0.5f},  {1.0f, 1.0f, 1.0f}},
            {{0.5f, 0.5f},   {1.0f, 1.0f, 1.0f}},
            {{0.5f, 0.5f},   {1.0f, 1.0f, 1.0f}},
            {{-0.5f, 0.5f},  {1.0f, 1.0f, 1.0f}},
            {{-0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}
        };
        
        std::shared_ptr<Model> model = std::make_shared<Model>(_device, vertecies);

        Entity square = _registry.create_entity();
        _registry.emplace_component<ModelComponent>(square, ModelComponent{model});
        _registry.emplace_component<ColorComponent>(square, ColorComponent{{0.0f, 1.0f, 0.0f}});
        _registry.emplace_component<TransformComponent>(square, TransformComponent{{ .2f, 0.0f }, {2.f, 2.f}, .25f * glm::two_pi<float>()});
        _registry.emplace_component<VelocityComponent>(square, VelocityComponent{{0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, 10.0f});
        _registry.emplace_component<GravityComponent>(square, GravityComponent{{0.0f, 9.81f}, true});
        

        
    }

    Application::~Application() {
        _unifiedRenderSystem.reset();
    }

}
