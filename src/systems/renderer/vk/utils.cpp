#include "utils.hpp"

#include "core/logger.hpp"

#include <cstring>
#include <fstream>

namespace rin::renderer::vulkan::utils {

static void log_supported_layers(VkLayerProperties*, size_t);
static void log_supported_extensions(VkExtensionProperties*, size_t);
static bool supports_required_layers(VkLayerProperties*, size_t, const darray<const char*>&);
static bool supports_required_extensions(VkExtensionProperties*, size_t, const darray<const char*>&);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool load_shader_module(VkDevice device, const char* filePath, VkShaderModule* out)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        return false;
    }

    // find what the size of the file is by looking up the location of the cursor
    // because the cursor is at the end, it gives the size directly in bytes
    size_t fileSize = (size_t)file.tellg();

    // spirv expects the buffer to be on uint32, so make sure to reserve an int vector big enough for the entire file
    darray<u32> buffer { fileSize / sizeof(u32), true };
    file.seekg(0);
    file.read((char*)buffer.data, fileSize);
    file.close();

    VkShaderModuleCreateInfo info {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = buffer.capacity * sizeof(u32),
        .pCode = buffer.data,
    };

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &info, nullptr, &shaderModule) != VK_SUCCESS) {
        return false;
    }

    *out = shaderModule;
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool load_instance_layers(VkInstanceCreateInfo* create_info, darray<const char*>& required_layers)
{
    u32 count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    VkLayerProperties* props = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * count);
    vkEnumerateInstanceLayerProperties(&count, props);
    log_supported_layers(props, count);

    if (!supports_required_layers(props, count, required_layers)) {
        free(props);
        return false;
    }

    create_info->enabledLayerCount = required_layers.len;
    create_info->ppEnabledLayerNames = required_layers.data;
    free(props);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool load_instance_extensions(VkInstanceCreateInfo* create_info, darray<const char*>& required_extensions)
{
    u32 count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    VkExtensionProperties* props = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, props);
    log_supported_extensions(props, count);

    if (!supports_required_extensions(props, count, required_extensions)) {
        free(props);
        return false;
    }

    create_info->enabledExtensionCount = required_extensions.len;
    create_info->ppEnabledExtensionNames = required_extensions.data;
    free(props);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool load_device_extensions(VkPhysicalDevice device, VkDeviceCreateInfo* create_info, darray<const char*>& required_extensions)
{
    u32 count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    VkExtensionProperties* props = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, props);
    log_supported_extensions(props, count);

    if (!supports_required_extensions(props, count, required_extensions)) {
        free(props);
        return false;
    }

    create_info->enabledExtensionCount = required_extensions.len;
    create_info->ppEnabledExtensionNames = required_extensions.data;
    free(props);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool supports_required_layers(VkLayerProperties* supported, size_t supported_len, const darray<const char*>& required)
{
    log::debug("Checking for layers support:");
    if (required.len == 0) {
        log::debug("No layer requested");
        return true;
    }

    bool all_supported = true;
    for (size_t i = 0; i < required.len; i++) {
        const char* req_lay = required[i];
        bool found = false;
        for (size_t j = 0; j < supported_len; j++) {
            if (strcmp(req_lay, supported[j].layerName) == 0) {
                log::debug("\t%s is supported", req_lay);
                found = true;
                break;
            }
        }

        if (!found) {
            log::warn("\t%s is not supported", req_lay);
            all_supported = false;
        }
    }

    return all_supported;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool supports_required_extensions(VkExtensionProperties* supported, size_t supported_len, const darray<const char*>& required)
{
    log::debug("Checking for extensions support:");
    if (required.len == 0) {
        log::debug("No extension requested");
        return true;
    }

    bool all_supported = true;
    for (size_t i = 0; i < required.len; i++) {
        const char* req_ext = required[i];
        bool found = false;
        for (size_t j = 0; j < supported_len; j++) {
            if (strcmp(req_ext, supported[j].extensionName) == 0) {
                log::debug("\t%s is supported", req_ext);
                found = true;
                break;
            }
        }

        if (!found) {
            log::warn("\t%s is not supported", req_ext);
            all_supported = false;
        }
    }

    return all_supported;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void log_supported_layers(VkLayerProperties* layers, size_t len)
{
    log::debug("Supported Layers:");
    for (size_t i = 0; i < len; i++) {
        log::debug("\t%s", layers[i].layerName);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void log_supported_extensions(VkExtensionProperties* extensions, size_t len)
{
    log::debug("Supported Extensions:");
    for (size_t i = 0; i < len; i++) {
        log::debug("\t%s", extensions[i].extensionName);
    }
}

}
