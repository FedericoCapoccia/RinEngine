#include "gui.hpp"

#include "core/logger.hpp"
#include "systems/window/window.hpp"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <cmath>
#include <imgui.h>

namespace rin::renderer::gui {

struct gui_t {
    ImGuiContext* context;
    VkDescriptorPool pool;
    VkDevice device;
};

static gui_t* state = nullptr;

static f32 linearize_color(f32 srgb)
{
    if (srgb <= 0.04045) {
        return srgb / 12.92;
    }

    return std::pow((srgb + 0.055) / 1.055, 2.4);
}

bool initialize(vulkan::context_t* vk_context)
{
    if (state != nullptr) {
        log::error("renderer::gui::initialize -> GUI system has been already initialized");
        return false;
    }

    state = (gui_t*)calloc(1, sizeof(gui_t));
    state->device = vk_context->device->logical_device;

    VkDescriptorPoolSize pool_size[11] {
        VkDescriptorPoolSize { .type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = 1000 },
        VkDescriptorPoolSize { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1000 },
        VkDescriptorPoolSize { .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = 1000 },
        VkDescriptorPoolSize { .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = 1000 },
        VkDescriptorPoolSize { .type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, .descriptorCount = 1000 },
        VkDescriptorPoolSize { .type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, .descriptorCount = 1000 },
        VkDescriptorPoolSize { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1000 },
        VkDescriptorPoolSize { .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1000 },
        VkDescriptorPoolSize { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, .descriptorCount = 1000 },
        VkDescriptorPoolSize { .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, .descriptorCount = 1000 },
        VkDescriptorPoolSize { .type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = 1000 },
    };

    VkDescriptorPoolCreateInfo pool_info {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000,
        .poolSizeCount = 11,
        .pPoolSizes = pool_size,
    };

    VkResult result = VK_SUCCESS;

    result = vkCreateDescriptorPool(state->device, &pool_info, nullptr, &state->pool);

    if (result != VK_SUCCESS) {
        log::error("renderer::gui::initialize -> failed to create descriptor pool");
        shutdown();
        return false;
    }

    state->context = ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.Fonts->AddFontFromFileTTF("resources/fonts/jetbrainsmono.ttf", 20);

    f32 xscale, yscale;
    window::get_scale(&xscale, &yscale);

    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark(&style);
    style.ScaleAllSizes(std::fmax(xscale, yscale));

    for (u32 i = 0; i < ImGuiCol_COUNT; i++) {
        ImVec4* col = &style.Colors[i];
        col->x = linearize_color(col->x);
        col->y = linearize_color(col->y);
        col->z = linearize_color(col->z);
    }

    window::init_imgui_vulkan();

    VkPipelineRenderingCreateInfo rendering_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = nullptr,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &vk_context->swapchain->format.format,
        .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
    };

    ImGui_ImplVulkan_InitInfo init_info {
        .ApiVersion = VK_API_VERSION_1_4,
        .Instance = vk_context->instance,
        .PhysicalDevice = vk_context->device->physical_device,
        .Device = vk_context->device->logical_device,
        .QueueFamily = (u32)vk_context->device->graphics_queue.family,
        .Queue = vk_context->device->graphics_queue.handle,
        .DescriptorPool = state->pool,
        .RenderPass = VK_NULL_HANDLE,
        .MinImageCount = vk_context->swapchain->min_image_count,
        .ImageCount = (u32)vk_context->swapchain->images.len,
        .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        .PipelineCache = VK_NULL_HANDLE,
        .Subpass = 0,
        .DescriptorPoolSize = 0,
        .UseDynamicRendering = true,
        .PipelineRenderingCreateInfo = rendering_info,
        .Allocator = nullptr,
        .CheckVkResultFn = nullptr,
        .MinAllocationSize = 1024 * 1024,
    };

    ImGui_ImplVulkan_Init(&init_info);
    ImGui_ImplVulkan_CreateFontsTexture();

    return true;
}

void shutdown(void)
{
    if (state == nullptr) {
        return;
    }

    ImGui_ImplVulkan_Shutdown();

    if (state->pool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(state->device, state->pool, nullptr);
    }

    window::shutdown_imgui();
    ImGui::DestroyContext(state->context);
    free(state);
    state = nullptr;
}

void on_resize(u32 min_image_count)
{
    ImGui_ImplVulkan_SetMinImageCount(min_image_count);
}

void prepare(void)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void draw(VkCommandBuffer cmd)
{
    ImGui::Render();
    ImDrawData* data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(data, cmd);
}

}
