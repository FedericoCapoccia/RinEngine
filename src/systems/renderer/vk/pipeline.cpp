#include "pipeline.hpp"

#include "core/logger.hpp"

#include <vulkan/vk_enum_string_helper.h>

namespace rin::renderer::vulkan {
pipeline_builder_t& pipeline_builder_t::disable_blending(void)
{
    m_blending_attachment.colorWriteMask = 0;
    m_blending_attachment.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
    m_blending_attachment.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
    m_blending_attachment.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
    m_blending_attachment.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
    m_blending_attachment.blendEnable = VK_FALSE;

    return *this;
}

pipeline_builder_t& pipeline_builder_t::set_shaders(VkShaderModule vertex, VkShaderModule fragment)
{
    m_shader_stages.clear();

    m_shader_stages.push(VkPipelineShaderStageCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertex,
        .pName = "main",
        .pSpecializationInfo = nullptr,
    });

    m_shader_stages.push(VkPipelineShaderStageCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragment,
        .pName = "main",
        .pSpecializationInfo = nullptr,
    });

    return *this;
}

pipeline_builder_t& pipeline_builder_t::set_input_topology(VkPrimitiveTopology topology)
{
    m_input_assembly.topology = topology;
    m_input_assembly.primitiveRestartEnable = VK_FALSE;

    return *this;
}

pipeline_builder_t& pipeline_builder_t::set_polygon_mode(VkPolygonMode mode)
{
    m_rasterizer.polygonMode = mode;
    m_rasterizer.lineWidth = 1.0f;

    return *this;
}

pipeline_builder_t& pipeline_builder_t::set_cull_mode(VkCullModeFlags mode, VkFrontFace face)
{
    m_rasterizer.cullMode = mode;
    m_rasterizer.frontFace = face;

    return *this;
}

pipeline_builder_t& pipeline_builder_t::set_multisampling_none(void)
{
    m_multisampling.sampleShadingEnable = VK_FALSE;
    m_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    m_multisampling.minSampleShading = 1.0f;
    m_multisampling.pSampleMask = nullptr,
    m_multisampling.alphaToCoverageEnable = VK_FALSE,
    m_multisampling.alphaToOneEnable = VK_FALSE;

    return *this;
}

pipeline_builder_t& pipeline_builder_t::set_color_attachment_format(VkFormat format)
{
    m_color_attachment_format = format;
    m_rendering.colorAttachmentCount = 1;
    m_rendering.pColorAttachmentFormats = &m_color_attachment_format;

    return *this;
}

pipeline_builder_t& pipeline_builder_t::set_depth_format(VkFormat format)
{
    m_rendering.depthAttachmentFormat = format;

    return *this;
}

pipeline_builder_t& pipeline_builder_t::disable_depthtest(void)
{
    m_depth_stencil.depthTestEnable = VK_FALSE;
    m_depth_stencil.depthWriteEnable = VK_FALSE;
    m_depth_stencil.depthCompareOp = VK_COMPARE_OP_NEVER;
    m_depth_stencil.depthBoundsTestEnable = VK_FALSE;
    m_depth_stencil.stencilTestEnable = VK_FALSE;
    m_depth_stencil.front = {};
    m_depth_stencil.back = {};
    m_depth_stencil.minDepthBounds = 0.0f;
    m_depth_stencil.maxDepthBounds = 1.0f;

    return *this;
}

void pipeline_builder_t::clear(void)
{
    m_input_assembly = VkPipelineInputAssemblyStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
        .primitiveRestartEnable = 0,
    };

    m_rasterizer = VkPipelineRasterizationStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = 0,
        .rasterizerDiscardEnable = 0,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = 0,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = 0,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };

    m_blending_attachment = VkPipelineColorBlendAttachmentState {
        .blendEnable = 0,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = 0,
    };

    m_multisampling = VkPipelineMultisampleStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = 0,
        .minSampleShading = 0.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = 0,
        .alphaToOneEnable = 0,
    };

    m_layout = VK_NULL_HANDLE;

    m_depth_stencil = VkPipelineDepthStencilStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthTestEnable = 0,
        .depthWriteEnable = 0,
        .depthCompareOp = VK_COMPARE_OP_NEVER,
        .depthBoundsTestEnable = 0,
        .stencilTestEnable = 0,
        .front = {
            .failOp = VK_STENCIL_OP_KEEP,
            .passOp = VK_STENCIL_OP_KEEP,
            .depthFailOp = VK_STENCIL_OP_KEEP,
            .compareOp = VK_COMPARE_OP_NEVER,
            .compareMask = 0,
            .writeMask = 0,
            .reference = 0,
        },
        .back = {
            .failOp = VK_STENCIL_OP_KEEP,
            .passOp = VK_STENCIL_OP_KEEP,
            .depthFailOp = VK_STENCIL_OP_KEEP,
            .compareOp = VK_COMPARE_OP_NEVER,
            .compareMask = 0,
            .writeMask = 0,
            .reference = 0,
        },
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 0.0f,
    };

    m_rendering = VkPipelineRenderingCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = nullptr,
        .viewMask = 0,
        .colorAttachmentCount = 0,
        .pColorAttachmentFormats = nullptr,
        .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
    };

    m_viewport = {};
    m_scissor = VkRect2D {};

    m_shader_stages.clear();
}

bool pipeline_builder_t::build(VkDevice device, VkPipeline* out)
{
    VkPipelineViewportStateCreateInfo viewport_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };

    VkPipelineColorBlendStateCreateInfo blending {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = 0,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &m_blending_attachment,
        .blendConstants = { 0, 0, 0, 0 },
    };

    VkPipelineVertexInputStateCreateInfo vertex_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr,
    };

    VkDynamicState dynamic_state[] {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamic {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamic_state,
    };

    VkGraphicsPipelineCreateInfo pipeline_info {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &m_rendering,
        .flags = 0,
        .stageCount = (u32)m_shader_stages.len,
        .pStages = m_shader_stages.data,
        .pVertexInputState = &vertex_state,
        .pInputAssemblyState = &m_input_assembly,
        .pTessellationState = nullptr,
        .pViewportState = &viewport_state,
        .pRasterizationState = &m_rasterizer,
        .pMultisampleState = &m_multisampling,
        .pDepthStencilState = &m_depth_stencil,
        .pColorBlendState = &blending,
        .pDynamicState = &dynamic,
        .layout = m_layout,
        .renderPass = VK_NULL_HANDLE,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
    };

    VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, out);
    if (result != VK_SUCCESS) {
        log::error("failed to create pipeline: %s", string_VkResult(result));
        clear();
        return false;
    }

    return true;
}

}
