#include "context.hpp"

#include "core/containers/darray.hpp"
#include "core/logger.hpp"
#include "device.hpp"
#include "loader.hpp"
#include "swapchain.hpp"
#include "systems/window/window.hpp"
#include "utils.hpp"

#include <cstdlib>
#include <vulkan/vk_enum_string_helper.h>

namespace rin::renderer::vulkan::context {

static context_t* context = nullptr;

static VkBool32 on_validation(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT*, void*);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool create(const char* app_name, bool enable_validation, context_t** out)
{

    if (context != nullptr) {
        log::error("vulkan::context::create -> a vulkan context already exists");
        return false;
    }

    log::info("creating vulkan context");
    VkResult vk_result = VK_SUCCESS;

    context = (context_t*)calloc(1, sizeof(context_t));
    context->validation = enable_validation;

    if (!load_core()) {
        log::error("vulkan::context::create -> failed to load core function pointers");
        return false;
    }

    log::debug("================ Instance Creation ================");
    {
        VkApplicationInfo app_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = app_name,
            .applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
            .pEngineName = "Rin",
            .engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
            .apiVersion = VK_API_VERSION_1_4,
        };

        VkInstanceCreateInfo instance_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = 0,
            .ppEnabledExtensionNames = nullptr,
        };

        darray<const char*> required_layers { true };
        darray<const char*> required_extensions { true };
        window::get_vulkan_extensions(required_extensions);

        if (context->validation) {
            required_layers.push("VK_LAYER_KHRONOS_validation");
            required_layers.push("VK_LAYER_KHRONOS_synchronization2");
            required_extensions.push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        if (!utils::load_instance_layers(&instance_info, required_layers)
            || !utils::load_instance_extensions(&instance_info, required_extensions)) {
            log::error("vulkan::context::initialize -> missing required layers or extensions");
            destroy();
            return false;
        }

        vk_result = vkCreateInstance(&instance_info, nullptr, &context->instance);
        if (vk_result != VK_SUCCESS) {
            log::error("vulkan::context::initialize -> failed to create instance: %s", string_VkResult(vk_result));
            destroy();
            return false;
        }

        load_instance(context->instance);

        if (context->validation) {
            log::debug("setting up debug messenger");
            VkDebugUtilsMessageTypeFlagsEXT m_type = 0;
            m_type |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
            m_type |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            m_type |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

            VkDebugUtilsMessageSeverityFlagsEXT m_severity = 0;
            m_severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            m_severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

            VkDebugUtilsMessengerCreateInfoEXT info = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .pNext = nullptr,
                .flags = 0,
                .messageSeverity = m_severity,
                .messageType = m_type,
                .pfnUserCallback = on_validation,
                .pUserData = nullptr,
            };

            vk_result = vkCreateDebugUtilsMessengerEXT(context->instance, &info, nullptr, &context->messenger);
            if (vk_result != VK_SUCCESS) {
                log::error("vulkan::context::initialize -> failed to create debug messenger: %s", string_VkResult(vk_result));
                destroy();
                return false;
            }
        }
    }
    log::debug("===================================================");

    log::debug("===================== Surface =====================");
    {
        if (!window::create_vulkan_surface(context->instance, &context->surface)) {
            destroy();
            return false;
        }
        log::debug("VkSurfaceKHR created");
    }
    log::debug("===================================================");

    log::debug("================= Device Creation =================");
    if (!device::create(context)) {
        log::error("vulkan::context::create -> failed to create vulkan_device");
        destroy();
        return false;
    }
    log::debug("===================================================");

    log::debug("==================== Swapchain ====================");
    {
        VkExtent2D extent {};
        window::get_size(&extent.width, &extent.height);

        if (!swapchain::create(context, extent)) {
            log::error("vulkan::context::create -> failed to create surface");
            destroy();
            return false;
        }
        log::debug("VKSwapchainKHR created");
    }
    log::debug("===================================================");

    VmaAllocatorCreateInfo vma_info = {
        .flags = 0,
        .physicalDevice = context->device->physical_device,
        .device = context->device->logical_device,
        .preferredLargeHeapBlockSize = 0,
        .pAllocationCallbacks = nullptr,
        .pDeviceMemoryCallbacks = nullptr,
        .pHeapSizeLimit = nullptr,
        .pVulkanFunctions = nullptr,
        .instance = context->instance,
        .vulkanApiVersion = VK_API_VERSION_1_4,
        .pTypeExternalMemoryHandleTypes = nullptr,
    };

    VmaVulkanFunctions vma_functions {};
    vmaImportVulkanFunctionsFromVolk(&vma_info, &vma_functions);
    vma_info.pVulkanFunctions = &vma_functions;

    vk_result = vmaCreateAllocator(&vma_info, &context->vma);

    if (vk_result != VK_SUCCESS) {
        log::error("vulkan_context_create -> failed to create Vulkan Memory Allocator: %s", string_VkResult(vk_result));
        destroy();
        return false;
    }

    *out = context;
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void destroy(void)
{
    if (context == nullptr) {
        return;
    }

    log::debug("destroying vulkan context");

    if (context->vma != nullptr) {
        log::debug("destroying vulkan memory allocator");
        vmaDestroyAllocator(context->vma);
    }

    if (context->swapchain != nullptr) {
        log::debug("destroying vulkan swapchain");
        swapchain::destroy();
    }

    if (context->device != nullptr) {
        log::debug("destroying vulkan device");
        device::destroy();
    }

    if (context->instance != VK_NULL_HANDLE) {
        if (context->surface != VK_NULL_HANDLE) {
            log::debug("destroying vulkan surface");
            vkDestroySurfaceKHR(context->instance, context->surface, nullptr);
        }

        if (context->messenger != VK_NULL_HANDLE) {
            log::debug("destroying vulkan debug messenger");
            vkDestroyDebugUtilsMessengerEXT(context->instance, context->messenger, nullptr);
        }

        log::debug("destroying vulkan instance");
        vkDestroyInstance(context->instance, nullptr);
    }

    free(context);
    context = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void begin_label(VkCommandBuffer cmd, const char* name, const vec4f_t& color)
{
    if (!context->validation) {
        return;
    }

    VkDebugUtilsLabelEXT label {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
        .pNext = nullptr,
        .pLabelName = name,
        .color = {
            color.r,
            color.g,
            color.b,
            color.a,
        },
    };

    vkCmdBeginDebugUtilsLabelEXT(cmd, &label);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void end_label(VkCommandBuffer cmd);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static VkBool32 on_validation(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    (void)pUserData;
    const char* message = "No Message!";
    if (pCallbackData != nullptr) {
        message = pCallbackData->pMessage;
    }

    const char* type_tag = "";
    switch (messageTypes) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
        type_tag = "general";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        type_tag = "performance";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
        type_tag = "validation";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
        type_tag = "address binding";
        break;
    default:
        type_tag = "unknown";
    }

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        log::error("[%s] Message: %s", type_tag, message);
    }

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        log::warn("[%s] Message: %s", type_tag, message);
    }

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        log::info("[%s] Message: %s", type_tag, message);
    }

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        log::debug("[%s] Message: %s", type_tag, message);
    }

    return VK_FALSE;
}

}
