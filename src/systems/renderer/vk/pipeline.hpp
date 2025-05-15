#pragma once

#include "core/containers/darray.hpp"

#include <volk.h>

namespace rin::renderer::vulkan {

struct pipeline_t {
    VkPipeline handle;
    VkPipelineLayout layout;
};

class pipeline_builder_t {
public:
    pipeline_builder_t(void) { clear(); }
    bool build(VkDevice device, VkPipeline* out);
    void clear(void);

    pipeline_builder_t& disable_blending(void);
    pipeline_builder_t& set_shaders(VkShaderModule vertex, VkShaderModule fragment);
    pipeline_builder_t& set_input_topology(VkPrimitiveTopology topology);
    pipeline_builder_t& set_polygon_mode(VkPolygonMode mode);
    pipeline_builder_t& set_cull_mode(VkCullModeFlags mode, VkFrontFace face);
    pipeline_builder_t& set_multisampling_none(void);
    pipeline_builder_t& set_color_attachment_format(VkFormat format);
    pipeline_builder_t& set_depth_format(VkFormat format);
    pipeline_builder_t& disable_depthtest(void);

private:
    darray<VkPipelineShaderStageCreateInfo> m_shader_stages { true };
    VkPipelineInputAssemblyStateCreateInfo m_input_assembly;
    VkPipelineColorBlendAttachmentState m_blending_attachment;
    VkPipelineDepthStencilStateCreateInfo m_depth_stencil;
    VkPipelineMultisampleStateCreateInfo m_multisampling;
    VkPipelineRasterizationStateCreateInfo m_rasterizer;
    VkPipelineRenderingCreateInfo m_rendering;
    VkFormat m_color_attachment_format;
    VkViewport m_viewport;
    VkRect2D m_scissor;
    VkPipelineLayout m_layout;
};

}
