#include "vk.h"
#include "vkutil.h"

#include <stdlib.h>
#include <string.h>

const char* const ENGINE_VK_VALIDATION_LAYERS[] = {
    "VK_LAYER_KHRONOS_validation"
};
const bool ENGINE_VK_VALIDATION_LAYER_ENABLED = true;

bool vklib_util_check_validation_layer_support(void)
{
    if (!ENGINE_VK_VALIDATION_LAYER_ENABLED)
        return true;

    uint32_t layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    VkLayerProperties* layers = malloc(sizeof(VkLayerProperties)*layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, layers);

    LOGDEBUG("Available Vulkan Layers: %d", layer_count);

    uint32_t satisfied_requirements = 0;
    uint32_t requirement_count = sizeof(ENGINE_VK_VALIDATION_LAYERS) / sizeof(ENGINE_VK_VALIDATION_LAYERS[0]);
    for (uint32_t i = 0; i < layer_count; i++)
    {
        VkLayerProperties layer = layers[i];
        LOGDEBUG("Vulkan Layer: %s", layer.layerName);

        for (size_t j = 0; j < requirement_count; j++)
        {
            if (strcmp(ENGINE_VK_VALIDATION_LAYERS[j], layer.layerName) == 0)
                satisfied_requirements++;
        }
    }

    free(layers);

    if (satisfied_requirements < requirement_count)
    {
        LOGERROR("Could not satisfy required instance layers, expected %d, got %d.", requirement_count, satisfied_requirements);
        return false;
    }

    return true;
}

const char** vklib_util_get_enabled_layers(uint32_t* count)
{
    *count = 0;
    if (!ENGINE_VK_VALIDATION_LAYER_ENABLED)
        return NULL;

    *count = sizeof(ENGINE_VK_VALIDATION_LAYERS) / sizeof(ENGINE_VK_VALIDATION_LAYERS[0]);
    return (const char**)ENGINE_VK_VALIDATION_LAYERS;
}

const char** vklib_util_get_required_extensions(uint32_t* count)
{
    const char** glfwExtensionNames = glfwGetRequiredInstanceExtensions(count);

    const char** required_extensions = malloc(sizeof(const char*) * (*count+1));
    memcpy(required_extensions, glfwExtensionNames, (*count) * sizeof(const char*));

    if (ENGINE_VK_VALIDATION_LAYER_ENABLED)
        required_extensions[(*count)++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    return required_extensions;
}

VkResult evkCreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void evkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func)
        func(instance, debugMessenger, pAllocator);
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        LOGINFO("[VULKAN] %s", pCallbackData->pMessage);
    else if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        LOGWARNING("[VULKAN] %s", pCallbackData->pMessage);
    else if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        LOGERROR("[VULKAN] %s", pCallbackData->pMessage);
    else
        LOGDEBUG("[VULKAN] %s", pCallbackData->pMessage);

    return VK_FALSE;
}

bool vklib_util_debugging_enabled(void)
{
    return ENGINE_VK_VALIDATION_LAYER_ENABLED;
}

bool vklib_util_init_debug_messenger(vk_data* vkd)
{
    assume(vkd, false);
    assume(vkd->instance, false);

    if (!ENGINE_VK_VALIDATION_LAYER_ENABLED)
        return true;

    VkDebugUtilsMessengerCreateInfoEXT create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = vkDebugCallback;
    create_info.pUserData = NULL;

    return evkCreateDebugUtilsMessengerEXT(vkd->instance, &create_info, NULL, &vkd->debug_messenger) == VK_SUCCESS;
}

void vklib_util_dispose_debug_messenger(vk_data* vkd)
{
    assume(vkd);
    assume(vkd->instance);

    if (!ENGINE_VK_VALIDATION_LAYER_ENABLED)
        return;

    evkDestroyDebugUtilsMessengerEXT(vkd->instance, vkd->debug_messenger, NULL);
}

VkSurfaceFormatKHR vklib_util_choose_swap_surface_format(VkSurfaceFormatKHR* formats, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
    {
        VkSurfaceFormatKHR format = formats[i];
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }

    return formats[0];
}

VkPresentModeKHR vklib_util_choose_swap_present_mode(VkPresentModeKHR* modes, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
    {
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            return modes[i];
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

#define VK_CLAMP(v,mn,mx) (v < mn) ? mn : ((v > mx) ? mx : v)

VkExtent2D vklib_util_choose_swap_extent(GLFWwindow* window, VkSurfaceCapabilitiesKHR capabilities)
{
    if (capabilities.currentExtent.width != (uint32_t)-1)
        return capabilities.currentExtent;

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    VkExtent2D extent = {
        VK_CLAMP(w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        VK_CLAMP(h, capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
    };

    return extent;
}
