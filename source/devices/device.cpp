#include "devices/device.hpp"

#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

namespace vulkan {

    Device::Device(Window &window) : _window{window} {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createCommandPool();
    }

    VkCommandPool Device::getCommandPool() {
        return _commandPool;
    }

    VkDevice Device::getDevice() {
        return _device;
    }

    VkSurfaceKHR Device::getSurface() {
        return _surface;
    }

    VkQueue Device::getGraphicsQueue() {
        return _graphicsQueue;
    }

    VkQueue Device::getPresentQueue() {
        return _presentQueue;
    }

    SwapChainSupportDetails Device::getSwapChainSupport() {
        return querySwapChainSupport(_physicalDevice);
    }

    QueueFamilyIndices Device::findPhysicalQueueFamilies() {
        return findQueueFamilies(_physicalDevice);
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
        (void) messageSeverity; // Unused at the moment, voided to prevent warning
        (void) messageType; // Unused at the moment, voided to prevent warning
        (void) pUserData; // Unused at the moment, voided to prevent warning
        std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) {
        PFN_vkCreateDebugUtilsMessengerEXT function = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (function != nullptr) {
            return function(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) {
        PFN_vkDestroyDebugUtilsMessengerEXT function = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (function != nullptr) {
            function(instance, debugMessenger, pAllocator);
        }
    }

    void Device::createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("Validation layers requested, but not available.");
        }

        VkApplicationInfo appInformation{};
        appInformation.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInformation.pApplicationName = "Vulkan Application";
        appInformation.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInformation.pEngineName = "BBKEngine";
        appInformation.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInformation.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInformation{};
        createInformation.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInformation.pApplicationInfo = &appInformation;

        std::vector<const char *> extensions = getRequiredExtensions();

        #ifdef __APPLE__
            extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        #endif

        createInformation.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInformation.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (enableValidationLayers) {
            createInformation.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInformation.ppEnabledLayerNames = validationLayers.data();
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInformation.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
        } else {
            createInformation.enabledLayerCount = 0;
            createInformation.pNext = nullptr;
        }

        #ifdef __APPLE__
            createInformation.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        #endif

        if (vkCreateInstance(&createInformation, nullptr, &_instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create instance.");
        }

        hasGflwRequiredInstanceExtensions();
    }

    void Device::pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support.");
        }

        std::cout << "Device count: " << deviceCount << std::endl;
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

        for (const VkPhysicalDevice &device : devices) {
            if (isDeviceSuitable(device)) {
                _physicalDevice = device;
                break;
            }
        }

        if (_physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find a suitable GPU.");
        }

        vkGetPhysicalDeviceProperties(_physicalDevice, &_properties);
        std::cout << "Physical device: " << _properties.deviceName << std::endl;
    }

    void Device::createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInformations;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInformation = {};
            queueCreateInformation.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInformation.queueFamilyIndex = queueFamily;
            queueCreateInformation.queueCount = 1;
            queueCreateInformation.pQueuePriorities = &queuePriority;
            queueCreateInformations.push_back(queueCreateInformation);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInformation{};
        createInformation.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInformation.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInformations.size());
        createInformation.pQueueCreateInfos = queueCreateInformations.data();

        createInformation.pEnabledFeatures = &deviceFeatures;
        createInformation.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInformation.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInformation.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInformation.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInformation.enabledLayerCount = 0;
        }

        if (vkCreateDevice(_physicalDevice, &createInformation, nullptr, &_device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device.");
        }

        vkGetDeviceQueue(_device, indices.graphicsFamily, 0, &_graphicsQueue);
        vkGetDeviceQueue(_device, indices.presentFamily, 0, &_presentQueue);
    }

    void Device::createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

        VkCommandPoolCreateInfo poolInformation{};
        poolInformation.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInformation.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
        poolInformation.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(_device, &poolInformation, nullptr, &_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void Device::createSurface() {
        _window.createWindowSurface(_instance, &_surface);
    }

    bool Device::isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    void Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;
    }

    void Device::setupDebugMessenger() {
        if (!enableValidationLayers) {
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    bool Device::checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName : validationLayers) {
            bool layerFound = false;
            for (const VkLayerProperties &layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) {
                return false;
            }
        }
        return true;
    }

    std::vector<const char *> Device::getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    void Device::hasGflwRequiredInstanceExtensions() {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        std::cout << "Available extensions:" << std::endl;
        std::unordered_set<std::string> available;
        for (const VkExtensionProperties &extension : extensions) {
            std::cout << "\t" << extension.extensionName << std::endl;
            available.insert(extension.extensionName);
        }

        std::cout << "Required extensions:" << std::endl;
        auto requiredExtensions = getRequiredExtensions();
        for (const char* &required : requiredExtensions) {
            std::cout << "\t" << required << std::endl;
            if (available.find(required) == available.end()) {
                throw std::runtime_error("Missing required glfw extension.");
            }
        }
    }

    bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const VkExtensionProperties &extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const VkQueueFamilyProperties &queueFamily : queueFamilies) {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
                indices.graphicsFamilyHasValue = true;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);
            if (queueFamily.queueCount > 0 && presentSupport) {
                indices.presentFamily = i;
                indices.presentFamilyHasValue = true;
            }
            if (indices.isComplete()) {
                break;
            }
            i++;
        }

        return indices;
    }

    SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
        }
        return details;
    }

    VkFormat Device::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("Failed to find supported format.");
    }

    uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("Failed to find suitable memory type.");
    }

    void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
        VkBufferCreateInfo bufferInformation{};
        bufferInformation.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInformation.size = size;
        bufferInformation.usage = usage;
        bufferInformation.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(_device, &bufferInformation, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create vertex buffer.");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(_device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInformation{};
        allocInformation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInformation.allocationSize = memRequirements.size;
        allocInformation.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(_device, &allocInformation, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate vertex buffer memory.");
        }

        vkBindBufferMemory(_device, buffer, bufferMemory, 0);
    }

    VkCommandBuffer Device::beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInformation{};
        allocInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInformation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInformation.commandPool = _commandPool;
        allocInformation.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(_device, &allocInformation, &commandBuffer);

        VkCommandBufferBeginInfo beginInformation{};
        beginInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInformation.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInformation);
        return commandBuffer;
    }

    void Device::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInformation{};
        submitInformation.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInformation.commandBufferCount = 1;
        submitInformation.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(_graphicsQueue, 1, &submitInformation, VK_NULL_HANDLE);
        vkQueueWaitIdle(_graphicsQueue);

        vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
    }

    void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void Device::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        endSingleTimeCommands(commandBuffer);
    }

    void Device::createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory) {
        if (vkCreateImage(_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(_device, image, &memRequirements);

        VkMemoryAllocateInfo allocInformation{};
        allocInformation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInformation.allocationSize = memRequirements.size;
        allocInformation.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(_device, &allocInformation, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        if (vkBindImageMemory(_device, image, imageMemory, 0) != VK_SUCCESS) {
            throw std::runtime_error("failed to bind image memory!");
        }
    }

    Device::~Device() {
        vkDestroyCommandPool(_device, _commandPool, nullptr);
        vkDestroyDevice(_device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkDestroyInstance(_instance, nullptr);
    }

}
