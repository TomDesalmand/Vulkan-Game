#pragma once

// Header files include //
#include "window.hpp"
#include "renderer.hpp"
#include "devices/device.hpp"
#include "game/ecs.hpp"
#include "systems/render_system.hpp"

// STD include //
#include <memory>

namespace vulkan {

    class Application {
        private:
            static constexpr int WIDTH = 1920;
            static constexpr int HEIGHT = 1080;
            Window _window{WIDTH, HEIGHT, "Vulkan Application"};
            Device _device{_window};
            Renderer _renderer{_window, _device};
            std::unique_ptr<RenderSystem> _renderSystem;
            Registry _registry;

            void loadObjects();

        public:
            Application();
            void run();
            ~Application();

            Application(const Application &) = delete;
            Application &operator=(const Application &) = delete;
    };

}