// Header Files Include
#include "input/input.hpp"

// STD include
#include <algorithm>

namespace vulkan {

    Input::Input(GLFWwindow* window) : _window(window) {
        _monitoredKeys = {
            GLFW_KEY_SPACE,
        };
        _monitoredMouseButtons = {
            GLFW_MOUSE_BUTTON_LEFT,
            GLFW_MOUSE_BUTTON_RIGHT
        };
        for (int key : _monitoredKeys) {
            _previousState[key] = false;
            _pressedThisFrame[key] = false;
        }
        for (int btn : _monitoredMouseButtons) {
            _previousMouseState[btn] = false;
            _pressedMouseThisFrame[btn] = false;
        }
    }
    
    void Input::update() {
        if (!_window) {
            return;
        }
        glfwGetCursorPos(_window, &_mouseX, &_mouseY);
        for (int key : _monitoredKeys) {
            int state = glfwGetKey(_window, key);
            bool down = (state == GLFW_PRESS || state == GLFW_REPEAT);
            bool pressed = (down && !_previousState[key]);
    
            _pressedThisFrame[key] = pressed;
            _previousState[key] = down;
        }
        for (int btn : _monitoredMouseButtons) {
            int state = glfwGetMouseButton(_window, btn);
            bool down = (state == GLFW_PRESS || state == GLFW_REPEAT);
            bool pressed = (down && !_previousMouseState[btn]);

            _pressedMouseThisFrame[btn] = pressed;
            _previousMouseState[btn] = down;
        }
    }
    
    bool Input::isKeyDown(int key) const {
        if (!_window) {
            return false;
        }
        int state = glfwGetKey(_window, key);
        return (state == GLFW_PRESS || state == GLFW_REPEAT);
    }
    
    bool Input::isKeyPressed(int key) const {
        auto iterator = _pressedThisFrame.find(key);
        if (iterator != _pressedThisFrame.end()) {
            return iterator->second;
        }
        return false;
    }

    bool Input::isMouseDown(int button) const {
        if (!_window) {
            return false;
        }
        int state = glfwGetMouseButton(_window, button);
        return (state == GLFW_PRESS || state == GLFW_REPEAT);
    }

    bool Input::isMousePressed(int button) const {
        auto it = _pressedMouseThisFrame.find(button);
        if (it != _pressedMouseThisFrame.end()) {
            return it->second;
        }
        return false;
    }

    std::pair<double, double> Input::getMousePosition() {
        return std::make_pair(_mouseX, _mouseY);
    }

}