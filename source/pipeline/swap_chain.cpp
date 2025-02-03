#include "pipeline/swap_chain.hpp"

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace vulkan {

    SwapChain::SwapChain(Device &deviceRef, VkExtent2D extent) : _device{deviceRef}, _windowExtent{extent} {
        init();
    }

    SwapChain::SwapChain(Device &deviceRef, VkExtent2D extent, std::shared_ptr<SwapChain> previous) : _device{deviceRef}, _windowExtent{extent}, _oldSwapChain{previous} {
        init();
        _oldSwapChain = nullptr;
    }

    void SwapChain::init() {
        createSwapChain();
        createImageViews();
        createRenderPass();
        createDepthResources();
        createFramebuffers();
        createSyncObjects();
    }

    VkFramebuffer SwapChain::getFrameBuffer(int index) {
        return _swapChainFramebuffers[index];
    }

    VkRenderPass SwapChain::getRenderPass() {
        return _renderPass;
    }

    VkImageView SwapChain::getImageView(int index) {
        return _swapChainImageViews[index];
    }

    size_t SwapChain::getImageCount() {
        return _swapChainImages.size();
    }

    VkFormat SwapChain::getSwapChainImageFormat() {
        return _swapChainImageFormat;
    }

    VkExtent2D SwapChain::getSwapChainExtent() {
        return _swapChainExtent;
    }

    uint32_t SwapChain::getWidth() {
        return _swapChainExtent.width;
    }

    uint32_t SwapChain::getHeight() {
        return _swapChainExtent.height;
    }

    float SwapChain::extentAspectRatio() {
        return static_cast<float>(_swapChainExtent.width) / static_cast<float>(_swapChainExtent.height);
    }

    VkResult SwapChain::acquireNextImage(uint32_t *imageIndex) {
        vkWaitForFences(_device.getDevice(), 1, &_inFlightFences[_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
        VkResult result = vkAcquireNextImageKHR(_device.getDevice(), _swapChain, std::numeric_limits<uint64_t>::max(), _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, imageIndex);
        return result;
    }

    VkResult SwapChain::submitCommandBuffers(
        const VkCommandBuffer *buffers, uint32_t *imageIndex) {
        if (_imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(_device.getDevice(), 1, &_imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
        }
        _imagesInFlight[*imageIndex] = _inFlightFences[_currentFrame];

        VkSubmitInfo submitInformation = {};
        submitInformation.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {_imageAvailableSemaphores[_currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInformation.waitSemaphoreCount = 1;
        submitInformation.pWaitSemaphores = waitSemaphores;
        submitInformation.pWaitDstStageMask = waitStages;

        submitInformation.commandBufferCount = 1;
        submitInformation.pCommandBuffers = buffers;

        VkSemaphore signalSemaphores[] = {_renderFinishedSemaphores[_currentFrame]};
        submitInformation.signalSemaphoreCount = 1;
        submitInformation.pSignalSemaphores = signalSemaphores;

        vkResetFences(_device.getDevice(), 1, &_inFlightFences[_currentFrame]);
        if (vkQueueSubmit(_device.getGraphicsQueue(), 1, &submitInformation, _inFlightFences[_currentFrame]) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInformation = {};
        presentInformation.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInformation.waitSemaphoreCount = 1;
        presentInformation.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {_swapChain};
        presentInformation.swapchainCount = 1;
        presentInformation.pSwapchains = swapChains;

        presentInformation.pImageIndices = imageIndex;

        VkResult result = vkQueuePresentKHR(_device.getPresentQueue(), &presentInformation);

        _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
    }

    void SwapChain::createSwapChain() {
        SwapChainSupportDetails swapChainSupport = _device.getSwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInformation = {};
        createInformation.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInformation.surface = _device.getSurface();
        createInformation.minImageCount = imageCount;
        createInformation.imageFormat = surfaceFormat.format;
        createInformation.imageColorSpace = surfaceFormat.colorSpace;
        createInformation.imageExtent = extent;
        createInformation.imageArrayLayers = 1;
        createInformation.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = _device.findPhysicalQueueFamilies();
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInformation.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInformation.queueFamilyIndexCount = 2;
            createInformation.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInformation.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInformation.queueFamilyIndexCount = 0;
            createInformation.pQueueFamilyIndices = nullptr;
        }

        createInformation.preTransform = swapChainSupport.capabilities.currentTransform;
        createInformation.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInformation.presentMode = presentMode;
        createInformation.clipped = VK_TRUE;
        createInformation.oldSwapchain = _oldSwapChain == nullptr ? VK_NULL_HANDLE : _oldSwapChain->_swapChain;

        if (vkCreateSwapchainKHR(_device.getDevice(), &createInformation, nullptr, &_swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(_device.getDevice(), _swapChain, &imageCount, nullptr);
        _swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(_device.getDevice(), _swapChain, &imageCount, _swapChainImages.data());

        _swapChainImageFormat = surfaceFormat.format;
        _swapChainExtent = extent;
    }

    void SwapChain::createImageViews() {
        _swapChainImageViews.resize(_swapChainImages.size());
        for (size_t i = 0; i < _swapChainImages.size(); i++) {
            VkImageViewCreateInfo viewInformation{};
            viewInformation.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInformation.image = _swapChainImages[i];
            viewInformation.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInformation.format = _swapChainImageFormat;
            viewInformation.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInformation.subresourceRange.baseMipLevel = 0;
            viewInformation.subresourceRange.levelCount = 1;
            viewInformation.subresourceRange.baseArrayLayer = 0;
            viewInformation.subresourceRange.layerCount = 1;

            if (vkCreateImageView(_device.getDevice(), &viewInformation, nullptr, &_swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create texture image view.");
            }
        }
    }

    void SwapChain::createRenderPass() {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = getSwapChainImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentReference = {};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentReference;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcAccessMask = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstSubpass = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInformation = {};
        renderPassInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInformation.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInformation.pAttachments = attachments.data();
        renderPassInformation.subpassCount = 1;
        renderPassInformation.pSubpasses = &subpass;
        renderPassInformation.dependencyCount = 1;
        renderPassInformation.pDependencies = &dependency;

        if (vkCreateRenderPass(_device.getDevice(), &renderPassInformation, nullptr, &_renderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass.");
        }
    }

    void SwapChain::createFramebuffers() {
        _swapChainFramebuffers.resize(getImageCount());
        for (size_t i = 0; i < getImageCount(); i++) {
            std::array<VkImageView, 2> attachments = {_swapChainImageViews[i], _depthImageViews[i]};
            VkExtent2D swapChainExtent = getSwapChainExtent();
            VkFramebufferCreateInfo framebufferInformation = {};
            framebufferInformation.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInformation.renderPass = _renderPass;
            framebufferInformation.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInformation.pAttachments = attachments.data();
            framebufferInformation.width = swapChainExtent.width;
            framebufferInformation.height = swapChainExtent.height;
            framebufferInformation.layers = 1;

            if (vkCreateFramebuffer(_device.getDevice(), &framebufferInformation, nullptr, &_swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create framebuffer.");
            }
        }
    }

    void SwapChain::createDepthResources() {
        VkFormat depthFormat = findDepthFormat();
        VkExtent2D swapChainExtent = getSwapChainExtent();

        _depthImages.resize(getImageCount());
        _depthImageMemories.resize(getImageCount());
        _depthImageViews.resize(getImageCount());

        for (size_t i = 0; i < _depthImages.size(); i++) {
            VkImageCreateInfo imageInformation{};
            imageInformation.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInformation.imageType = VK_IMAGE_TYPE_2D;
            imageInformation.extent.width = swapChainExtent.width;
            imageInformation.extent.height = swapChainExtent.height;
            imageInformation.extent.depth = 1;
            imageInformation.mipLevels = 1;
            imageInformation.arrayLayers = 1;
            imageInformation.format = depthFormat;
            imageInformation.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInformation.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInformation.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInformation.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInformation.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInformation.flags = 0;

            _device.createImageWithInfo(imageInformation, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImages[i], _depthImageMemories[i]);

            VkImageViewCreateInfo viewInformation{};
            viewInformation.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInformation.image = _depthImages[i];
            viewInformation.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInformation.format = depthFormat;
            viewInformation.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInformation.subresourceRange.baseMipLevel = 0;
            viewInformation.subresourceRange.levelCount = 1;
            viewInformation.subresourceRange.baseArrayLayer = 0;
            viewInformation.subresourceRange.layerCount = 1;

            if (vkCreateImageView(_device.getDevice(), &viewInformation, nullptr, &_depthImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create texture image view.");
            }
        }
    }

    void SwapChain::createSyncObjects() {
        _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        _imagesInFlight.resize(getImageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInformation = {};
        semaphoreInformation.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInformation = {};
        fenceInformation.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInformation.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(_device.getDevice(), &semaphoreInformation, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS || vkCreateSemaphore(_device.getDevice(), &semaphoreInformation, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS || vkCreateFence(_device.getDevice(), &fenceInformation, nullptr, &_inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create synchronization objects for a frame.");
            }
        }
    }

    VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const VkSurfaceFormatKHR &availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        // High power consumption but low latency //
        for (const VkPresentModeKHR &availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                std::cout << "Buffer swap mode: Mailbox" << std::endl;
                return availablePresentMode;
            }
        }

        // High power consumption and tearing but low latency //
        for (const VkPresentModeKHR &availablePresentMode : availablePresentModes) {
          if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            std::cout << "Buffer swap mode: Immediate" << std::endl;
            return availablePresentMode;
          }
        }

        // Low power consumption but more latency (better for mobile games) //
        std::cout << "Buffer swap mode: V-Sync" << std::endl;
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            VkExtent2D actualExtent = _windowExtent;
            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
            return actualExtent;
        }
    }

    VkFormat SwapChain::findDepthFormat() {
        return _device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    SwapChain::~SwapChain() {
        for (VkImageView imageView : _swapChainImageViews) {
            vkDestroyImageView(_device.getDevice(), imageView, nullptr);
        }

        _swapChainImageViews.clear();

        if (_swapChain != nullptr) {
            vkDestroySwapchainKHR(_device.getDevice(), _swapChain, nullptr);
            _swapChain = nullptr;
        }

        for (size_t i = 0; i < _depthImages.size(); i++) {
            vkDestroyImageView(_device.getDevice(), _depthImageViews[i], nullptr);
            vkDestroyImage(_device.getDevice(), _depthImages[i], nullptr);
            vkFreeMemory(_device.getDevice(), _depthImageMemories[i], nullptr);
        }

        for (VkFramebuffer framebuffer : _swapChainFramebuffers) {
            vkDestroyFramebuffer(_device.getDevice(), framebuffer, nullptr);
        }

        vkDestroyRenderPass(_device.getDevice(), _renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(_device.getDevice(), _renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(_device.getDevice(), _imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(_device.getDevice(), _inFlightFences[i], nullptr);
        }
    }

}
