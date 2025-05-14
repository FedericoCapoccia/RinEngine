#include "renderer.hpp"

#include "core/logger.hpp"
#include "vk/context.hpp"
#include "vk/types.hpp"

namespace rin::renderer {

#ifdef RRELEASE
static const bool ENABLE_VALIDATION = false;
#else
static const bool ENABLE_VALIDATION = true;
#endif

struct state_t {
    vulkan::context_t* context;
};

struct state_t* state = nullptr;

/*
 * [14:56:52] [err]        [validation]. Message:
 vkQueueSubmit2(): pSubmits[0].pSignalSemaphoreInfos[0].semaphore (VkSemaphore 0x1a000000001a[RenderDone[0]]) is being signaled by
VkQueue 0x13b06695f10, but it may still be in use by VkSwapchainKHR 0x30000000003.
Here are the most recently acquired image indices: [0], 1, 2.
(brackets mark the last use of VkSemaphore 0x1a000000001a[RenderDone[0]] in a presentation operation)
Swapchain image 0 was presented but was not re-acquired, so VkSemaphore 0x1a000000001a[RenderDone[0]] may still be in use and canno
t be safely reused with image index 2.
Vulkan insight: One solution is to assign each image its own semaphore. Here are some common methods to ensure that a semaphore pas
sed to vkQueuePresentKHR is not in use and can be safely reused:
        a) Use a separate semaphore per swapchain image. Index these semaphores using the index of the acquired image.
        b) Consider the VK_EXT_swapchain_maintenance1 extension. It allows using a VkFence with the presentation operation.
The Vulkan spec states: The semaphore member of any binary semaphore element of the pSignalSemaphoreInfos member of any element of
pSubmits must be unsignaled when the semaphore signal operation it defines is executed on the device (https://vulkan.lunarg.com/doc
/view/1.4.313.0/windows/antora/spec/latestchapters/cmdbuffers.html#VUID-vkQueueSubmit2-semaphore-03868)
 * */
/*
 * This means that I have a single ImageAcquired semaphore and as many RenderDone semaphore as the swapchain images count I guess
 */
bool initialize(const char* app_name)
{
    if (state != nullptr) {
        log::error("renderer::initialize -> renderer system has been already initialized");
        return false;
    }

    state = (state_t*)calloc(1, sizeof(state_t));

    if (!vulkan::context::create(app_name, ENABLE_VALIDATION, &state->context)) {
        log::error("renderer::initialize -> failed to create vulkan context");
        shutdown();
        return false;
    }

    return true;
}

void shutdown(void)
{
    if (state == nullptr) {
        return;
    }

    vulkan::context::destroy();
    state->context = nullptr;

    free(state);
    state = nullptr;
}

}
