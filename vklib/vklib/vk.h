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

typedef struct
{
    VkInstance instance;
} vk_data;

vk_data vklib_init(GLFWwindow* window);
void vklib_dispose(vk_data* vkd);

#endif
