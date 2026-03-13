#ifndef __VKLIB__VK_H_
#define __VKLIB__VK_H_

#include <volk.h>

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef WIN32
#   ifdef VKLIB_BUILD
#       define VKLIBAPI __declspec(dllexport)
#   else
#       define VKLIBAPI __declspec(dllimport)
#   endif
#   define __FILE_NAME__ __FILE__
#else
#   define VKLIBAPI
#endif

#define LOGLEVEL_INFO       (uint8_t)0
#define LOGLEVEL_DEBUG      (uint8_t)1
#define LOGLEVEL_ERROR      (uint8_t)2
#define LOGLEVEL_WARNING    (uint8_t)3

#define LOG(level, ...) vklib_log_msg(level, vklib_strfmt(__VA_ARGS__), __FILE_NAME__, __LINE__)
#define LOGINFO(...) vklib_log_msg(LOGLEVEL_INFO, vklib_strfmt(__VA_ARGS__), __FILE_NAME__, __LINE__)
#define LOGDEBUG(...) vklib_log_msg(LOGLEVEL_DEBUG, vklib_strfmt(__VA_ARGS__), __FILE_NAME__, __LINE__)
#define LOGERROR(...) vklib_log_msg(LOGLEVEL_ERROR, vklib_strfmt(__VA_ARGS__), __FILE_NAME__, __LINE__)
#define LOGWARNING(...) vklib_log_msg(LOGLEVEL_WARNING, vklib_strfmt(__VA_ARGS__), __FILE_NAME__, __LINE__)

#define assume(expr, ...) if (!(expr)) { LOGERROR("assume '%s' failed", #expr); return __VA_ARGS__; } (void)0

typedef void(*vklib_log_msg_t)(uint8_t lvl, const char* msg, const char* file, int line);

typedef struct
{
    VkDebugUtilsMessengerEXT debug_messenger;
    VkInstance instance;

    VkPhysicalDevice physical_device;
    VkDevice device;

    VkQueue graphics_queue;
    VkQueue presentation_queue;

    VkSurfaceKHR surface;
    
    VkImageView* swapchain_image_views;
    uint32_t swapchain_image_count;
    VkImage* swapchain_images;

    VkExtent2D swapchain_extent;
    VkFormat swapchain_format;
    VkSwapchainKHR swapchain;
} vk_data;

extern vklib_log_msg_t vklib_log_msg;

VKLIBAPI vk_data vklib_init(GLFWwindow* window, vklib_log_msg_t logfunc);
VKLIBAPI void vklib_dispose(vk_data* vkd);

const char* vklib_strfmt(const char* fmt, ...);

#endif
