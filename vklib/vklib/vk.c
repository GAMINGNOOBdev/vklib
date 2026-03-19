#include "vklib/vk.h"
#include "vklib/vkdev.h"
#include "vklib/vkutil.h"
#include "vklib/vkdisplay.h"
#include "vklib/vkframebuffer.h"

#include <vulkan/vulkan_core.h>

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

void vklib_log_msg_fallback(uint8_t lvl, const char* msg, const char* file, int line){}

vklib_log_msg_t vklib_log_msg = vklib_log_msg_fallback;

const char* vklib_strfmt(const char* fmt, ...)
{
    static char mLoggingFormattingBuffer[4096];

    va_list args;
    va_start(args, fmt);
    vsnprintf(mLoggingFormattingBuffer, 4096, fmt, args);
    va_end(args);

    return mLoggingFormattingBuffer;
}

extern VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

vklibd vklib_init(vklibd_init_data init_data)
{
    vklibd vkd = {};
    if (!init_data.window || !init_data.logfunc)
        return vkd;

    if (init_data.logfunc != NULL)
        vklib_log_msg = init_data.logfunc;

    if (volkInitialize() != VK_SUCCESS)
        return vkd;

    assume(vklib_util_check_validation_layer_support(), vkd);

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = init_data.app_name ? init_data.app_name : "vulkan abstraction layer";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = init_data.engine_name ? init_data.engine_name : "none";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    uint32_t tmp_count = 0;
    create_info.ppEnabledExtensionNames = vklib_util_get_required_extensions(&tmp_count);
    create_info.enabledExtensionCount = tmp_count;
    create_info.ppEnabledLayerNames = vklib_util_get_enabled_layers(&tmp_count);
    create_info.enabledLayerCount = tmp_count;

    if (vklib_util_debugging_enabled())
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo2 = {};
        createInfo2.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo2.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo2.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo2.pfnUserCallback = vkDebugCallback;
        createInfo2.pUserData = NULL;
        create_info.pNext = &createInfo2;
    }

    if (vkCreateInstance(&create_info, NULL, &vkd.instance) != VK_SUCCESS)
    {
        LOGERROR("cannot create vulkan instance, exiting");
        return vkd;
    }
    free((void*)create_info.ppEnabledExtensionNames);
    volkLoadInstance(vkd.instance);
    vkd.window = init_data.window;

    if (!vklib_util_init_debug_messenger(&vkd))
    {
        LOGERROR("cannot init vulkan debugging for this instance, exiting");
        return vkd;
    }

    // presentation setup
    if (glfwCreateWindowSurface(vkd.instance, vkd.window, NULL, &vkd.surface) != VK_SUCCESS)
    {
        LOGERROR("could not create a window surface");
        return vkd;
    }

    if (!vklib_dev_pick(&vkd))
    {
        LOGERROR("could not find a valid vulkan compatible gpu");
        return vkd;
    }

    if (!vklib_display_create_swapchain(&vkd))
    {
        LOGERROR("failed to create swap chain");
        return vkd;
    }


    LOGINFO("Initialized vulkan");

    return vkd;
}

VKLIBAPI void vklib_handle_view_changes(vklibd* vkd)
{
    assume(vkd && vkd->instance);

    int width = 0, height = 0;
    glfwGetFramebufferSize(vkd->window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(vkd->window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(vkd->device);

    vklib_framebuffers_destroy(vkd);
    vklib_display_destroy_swapchain(vkd);

    vklib_display_create_swapchain(vkd);
    vklib_framebuffers_init(vkd, vkd->last_render_pass);
}

void vklib_framebuffers_init(vklibd* vkd, VkRenderPass render_pass)
{
    assume(vkd && vkd->instance);

    vkd->last_render_pass = render_pass;

    vkd->framebuffers = malloc(sizeof(VkFramebuffer) * vkd->swapchain_image_count);
    vklib_framebuffer_create(vkd, render_pass, vkd->swapchain_image_count, vkd->framebuffers, vkd->swapchain_image_views);
}

void vklib_framebuffers_destroy(vklibd* vkd)
{
    assume(vkd);

    vklib_framebuffer_destroy(vkd, vkd->swapchain_image_count, vkd->framebuffers);
    free(vkd->framebuffers);
}

void vklib_destroy(vklibd* vkd)
{
    assume(vkd && vkd->instance);

    vklib_framebuffers_destroy(vkd);

    vklib_display_destroy_swapchain(vkd);

    assume(vklib_dev_destroy(vkd));
    vklib_util_destroy_debug_messenger(vkd);

    vkDestroySurfaceKHR(vkd->instance, vkd->surface, NULL);
    vkDestroyInstance(vkd->instance, NULL);

    LOGINFO("Disposed vulkan");
}
