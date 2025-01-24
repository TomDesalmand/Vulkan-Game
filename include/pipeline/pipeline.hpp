#pragma once

#include "../devices/device.hpp"
#include <string>
#include <vector>

namespace vulkan {

    struct PipelineConfigurationInformation {

        VkPipelineViewportStateCreateInfo viewportInformation;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInformation;
        VkPipelineRasterizationStateCreateInfo rasterizationInformation;
        VkPipelineMultisampleStateCreateInfo multisampleInformation;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInformation;
        VkPipelineDepthStencilStateCreateInfo depthStencilInformation;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInformation;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class Pipeline {
        private:
            Device& _device;
            VkPipeline _graphicsPipeline;
            VkShaderModule _vertShaderModule;
            VkShaderModule _fragShaderModule;

            static std::vector<char> readFile(const std::string &filePath);
            void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

        public:
            Pipeline(Device &device, const std::string &vertFilepath, const std::string &fragFilepath, const PipelineConfigurationInformation &configurationInformation);
            static void defaultPipelineConfigurationInformation(PipelineConfigurationInformation &configurationInformation);
            void bind(VkCommandBuffer commandbuffer);
            ~Pipeline();

            // Remove the copy operators to prevent make copies //
            Pipeline(const Pipeline &) = delete;
            Pipeline &operator=(const Pipeline &) = delete;

    };
}