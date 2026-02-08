#pragma once

// Header files include //
#include "window.hpp"
#include "renderer.hpp"
#include "devices/device.hpp"
#include "game/ecs.hpp"
#include "systems/render_system.hpp"

// STD include //
#include <memory>
#include <chrono>

namespace vulkan {
    
    using clock = std::chrono::high_resolution_clock;
    using time = std::chrono::high_resolution_clock::time_point;
    
    constexpr double TICKS_PER_SECOND = 128.0;
    constexpr double SECONDS_PER_TICK = 1.0 / TICKS_PER_SECOND;

    class Application {
        private:
            static constexpr int WIDTH = 1920;
            static constexpr int HEIGHT = 1080;
            Window _window{WIDTH, HEIGHT, "Vulkan Application"};
            Device _device{_window};
            Renderer _renderer{_window, _device};
            std::unique_ptr<UnifiedRenderSystem> _unifiedRenderSystem;
            Registry _registry;

            void loadObjects();
            void updateClock(time &previousTime, double &accumulator);

        public:
            Application();
            void run();
            ~Application();

            Application(const Application &) = delete;
            Application &operator=(const Application &) = delete;
    };

}