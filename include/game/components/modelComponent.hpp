#pragma once

// Header files include //
#include "pipeline/model.hpp"

// STD include //
#include <memory>

namespace vulkan {
    
    struct ModelComponent {
        std::shared_ptr<Model> model{};
    };
    
}