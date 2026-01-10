#pragma once

// GLFW for key constants and window pointer
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// STD includes
#include <unordered_map>
#include <vector>

namespace vulkan {

    class Input {
        private:
            GLFWwindow* _window;
            std::unordered_map<int, bool> _previousState;
            std::unordered_map<int, bool> _pressedThisFrame;
            std::vector<int> _monitoredKeys;

            std::unordered_map<int, bool> _previousMouseState;
            std::unordered_map<int, bool> _pressedMouseThisFrame;
            std::vector<int> _monitoredMouseButtons;

            double _mouseX = 0.0;
            double _mouseY = 0.0;
    
        public:
            explicit Input(GLFWwindow* window);
            void update();
            bool isKeyDown(int key) const;
            bool isKeyPressed(int key) const;
            bool isMouseDown(int button) const;
            bool isMousePressed(int button) const;
            std::pair<double, double> getMousePosition();
    };

}