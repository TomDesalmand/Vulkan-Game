// Header files include //
#include "window/application.hpp"
#include "logger/logging.hpp"

// STD include //
#include <cstdlib>

int main() {
    LOG_DEFINE_TYPE_LABELED(INFO,  1,   0, 200, 100, "INFO");
    LOG_DEFINE_TYPE_LABELED(ERROR, 0, 200,   0,   0, "ERROR");
    LOG_DEFINE_TYPE_LABELED(DEBUG, 2, 150, 150, 150, "DEBUG");
    LOG_DEFINE_TYPE_LABELED(WARN,  1, 255, 165,   0, "WARN");
    vulkan::Application application{};
    try {
        application.run();
    } catch(const std::exception &error) {
       ERROR("Failed to run application: ", error.what());
       return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
