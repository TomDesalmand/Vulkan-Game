#include "application.hpp"
#include <cstdlib>
#include <stdexcept>
#include <iostream>

int main()
{
    vulkan::Application application{};
    try {
        application.run();
    } catch(const std::exception &error) {
       std::cerr << "Failed to run application: " << error.what() << std::endl;
       return EXIT_FAILURE; 
    }
    return EXIT_SUCCESS;
}