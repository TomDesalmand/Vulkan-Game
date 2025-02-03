#include "pipeline/pipeline.hpp"
#include "pipeline/model.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cassert>

namespace vulkan {

    Pipeline::Pipeline(Device &device, const std::string &vertFilepath, const std::string &fragFilepath, const PipelineConfigurationInformation &configurationInformation) : _device{device} {
        assert(configurationInformation.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no pipelineLayout in configurationInformation.");
        assert(configurationInformation.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no renderPass in configurationInformation.");

        std::vector<char> vertCode = readFile(vertFilepath);
        std::vector<char> fragCode = readFile(fragFilepath);

        createShaderModule(vertCode, &_vertShaderModule);
        createShaderModule(fragCode, &_fragShaderModule);

        VkPipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = _vertShaderModule;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;
        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = _fragShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        std::vector<VkVertexInputBindingDescription> bindingDescriptions = Model::Vertex::getBindingDescriptions();
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions = Model::Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInformation{};
        vertexInputInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInformation.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInformation.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInformation.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputInformation.pVertexBindingDescriptions = bindingDescriptions.data();

        VkGraphicsPipelineCreateInfo pipelineInformation{};
        pipelineInformation.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInformation.stageCount = 2; // Count of how many programmable stages the pipeline will use. (shaders) //
        pipelineInformation.pStages = shaderStages;
        pipelineInformation.pVertexInputState = &vertexInputInformation;
        pipelineInformation.pInputAssemblyState = &configurationInformation.inputAssemblyInformation;
        pipelineInformation.pViewportState = &configurationInformation.viewportInformation;
        pipelineInformation.pRasterizationState = &configurationInformation.rasterizationInformation;
        pipelineInformation.pMultisampleState = &configurationInformation.multisampleInformation;
        pipelineInformation.pColorBlendState = &configurationInformation.colorBlendInformation;
        pipelineInformation.pDepthStencilState = &configurationInformation.depthStencilInformation;
        pipelineInformation.pDynamicState = &configurationInformation.dynamicStateInformation;
        pipelineInformation.layout = configurationInformation.pipelineLayout;
        pipelineInformation.renderPass = configurationInformation.renderPass;
        pipelineInformation.subpass = configurationInformation.subpass;
        pipelineInformation.basePipelineIndex = -1;
        pipelineInformation.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(_device.getDevice(), VK_NULL_HANDLE, 1, &pipelineInformation, nullptr, &_graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create graphics pipeline.");
        }
    }

    void Pipeline::createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule) {
        VkShaderModuleCreateInfo createInformation{};
        createInformation.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInformation.codeSize = code.size();
        createInformation.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(_device.getDevice(), &createInformation, nullptr, shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module.");
        }
    }

    std::vector<char> Pipeline::readFile(const std::string &filePath) {
        std::ifstream file{filePath, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    void Pipeline::defaultPipelineConfigurationInformation(PipelineConfigurationInformation &configurationInformation) {

        configurationInformation.inputAssemblyInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configurationInformation.inputAssemblyInformation.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configurationInformation.inputAssemblyInformation.primitiveRestartEnable = VK_FALSE;

        configurationInformation.viewportInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configurationInformation.viewportInformation.viewportCount = 1;
        configurationInformation.viewportInformation.pViewports = nullptr;
        configurationInformation.viewportInformation.scissorCount = 1;
        configurationInformation.viewportInformation.pScissors = nullptr;

        configurationInformation.rasterizationInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configurationInformation.rasterizationInformation.depthClampEnable = VK_FALSE;
        configurationInformation.rasterizationInformation.rasterizerDiscardEnable = VK_FALSE;
        configurationInformation.rasterizationInformation.polygonMode = VK_POLYGON_MODE_FILL;
        configurationInformation.rasterizationInformation.lineWidth = 1.0f;
        configurationInformation.rasterizationInformation.cullMode = VK_CULL_MODE_NONE;
        configurationInformation.rasterizationInformation.frontFace = VK_FRONT_FACE_CLOCKWISE;
        configurationInformation.rasterizationInformation.depthBiasEnable = VK_FALSE;
        configurationInformation.rasterizationInformation.depthBiasConstantFactor = 0.0f;
        configurationInformation.rasterizationInformation.depthBiasClamp = 0.0f;
        configurationInformation.rasterizationInformation.depthBiasSlopeFactor = 0.0f;

        configurationInformation.multisampleInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configurationInformation.multisampleInformation.sampleShadingEnable = VK_FALSE;
        configurationInformation.multisampleInformation.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configurationInformation.multisampleInformation.minSampleShading = 1.0f;
        configurationInformation.multisampleInformation.pSampleMask = nullptr;
        configurationInformation.multisampleInformation.alphaToCoverageEnable = VK_FALSE;
        configurationInformation.multisampleInformation.alphaToOneEnable = VK_FALSE;

        configurationInformation.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        configurationInformation.colorBlendAttachment.blendEnable = VK_FALSE;
        configurationInformation.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        configurationInformation.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        configurationInformation.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        configurationInformation.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        configurationInformation.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        configurationInformation.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        configurationInformation.colorBlendInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configurationInformation.colorBlendInformation.logicOpEnable = VK_FALSE;
        configurationInformation.colorBlendInformation.logicOp = VK_LOGIC_OP_COPY;
        configurationInformation.colorBlendInformation.attachmentCount = 1;
        configurationInformation.colorBlendInformation.pAttachments = &configurationInformation.colorBlendAttachment;
        configurationInformation.colorBlendInformation.blendConstants[0] = 0.0f;
        configurationInformation.colorBlendInformation.blendConstants[1] = 0.0f;
        configurationInformation.colorBlendInformation.blendConstants[2] = 0.0f;
        configurationInformation.colorBlendInformation.blendConstants[3] = 0.0f;

        configurationInformation.depthStencilInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configurationInformation.depthStencilInformation.depthTestEnable = VK_TRUE;
        configurationInformation.depthStencilInformation.depthWriteEnable = VK_TRUE;
        configurationInformation.depthStencilInformation.depthCompareOp = VK_COMPARE_OP_LESS;
        configurationInformation.depthStencilInformation.depthBoundsTestEnable = VK_FALSE;
        configurationInformation.depthStencilInformation.minDepthBounds = 0.0f;
        configurationInformation.depthStencilInformation.maxDepthBounds = 1.0f;
        configurationInformation.depthStencilInformation.stencilTestEnable = VK_FALSE;
        configurationInformation.depthStencilInformation.front = {};
        configurationInformation.depthStencilInformation.back = {};

        configurationInformation.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        configurationInformation.dynamicStateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        configurationInformation.dynamicStateInformation.pDynamicStates = configurationInformation.dynamicStateEnables.data();
        configurationInformation.dynamicStateInformation.dynamicStateCount = static_cast<uint32_t>(configurationInformation.dynamicStateEnables.size());
        configurationInformation.dynamicStateInformation.flags = 0;
    }

    void Pipeline::bind(VkCommandBuffer commandbuffer) {
        vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);
    }

    Pipeline::~Pipeline() {
        vkDestroyShaderModule(_device.getDevice(), _vertShaderModule, nullptr);
        vkDestroyShaderModule(_device.getDevice(), _fragShaderModule, nullptr);
        vkDestroyPipeline(_device.getDevice(), _graphicsPipeline, nullptr);
    }

}
