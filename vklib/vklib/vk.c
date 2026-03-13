#include "vk.h"
#include "vkdev.h"
#include "vkutil.h"
#include "vkdisplay.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

vklib_log_msg_t vklib_log_msg = NULL;

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

vk_data vklib_init(GLFWwindow* window, vklib_log_msg_t logfunc)
{
    vk_data vkd = {};
    if (!window || !logfunc)
        return vkd;

    vklib_log_msg = logfunc;

    if (volkInitialize() != VK_SUCCESS)
        return vkd;

    assume(vklib_util_check_validation_layer_support(), vkd);

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "vulkan abstraction layer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "none";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t tmp_count = 0;
    createInfo.ppEnabledExtensionNames = vklib_util_get_required_extensions(&tmp_count);
    createInfo.enabledExtensionCount = tmp_count;
    createInfo.ppEnabledLayerNames = vklib_util_get_enabled_layers(&tmp_count);
    createInfo.enabledLayerCount = tmp_count;

    if (vklib_util_debugging_enabled())
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo2 = {};
        createInfo2.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo2.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo2.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo2.pfnUserCallback = vkDebugCallback;
        createInfo2.pUserData = NULL;
        createInfo.pNext = &createInfo2;
    }

    if (vkCreateInstance(&createInfo, NULL, &vkd.instance) != VK_SUCCESS)
    {
        LOGERROR("cannot create vulkan instance, exiting");
        return vkd;
    }
    free((void*)createInfo.ppEnabledExtensionNames);
    volkLoadInstance(vkd.instance);

    if (!vklib_util_init_debug_messenger(&vkd))
    {
        LOGERROR("cannot init vulkan debugging for this instance, exiting");
        return vkd;
    }

    // presentation setup
    if (glfwCreateWindowSurface(vkd.instance, window, NULL, &vkd.surface) != VK_SUCCESS)
    {
        LOGERROR("could not create a window surface");
        return vkd;
    }

    if (!vklib_dev_pick(&vkd))
    {
        LOGERROR("could not find a valid vulkan compatible gpu");
        return vkd;
    }

    if (!vklib_display_create_swapchain(window, &vkd))
    {
        LOGERROR("failed to create swap chain");
        return vkd;
    }

    LOGINFO("Initialized vulkan");

    return vkd;
}

void vklib_dispose(vk_data* vkd)
{
    assume(vkd);
    assume(vkd->instance);

    vklib_display_dispose_swapchain(vkd);

    assume(vklib_dev_dispose(vkd));
    vklib_util_dispose_debug_messenger(vkd);

    vkDestroySurfaceKHR(vkd->instance, vkd->surface, NULL);
    vkDestroyInstance(vkd->instance, NULL);

    LOGINFO("Disposed vulkan");
}
