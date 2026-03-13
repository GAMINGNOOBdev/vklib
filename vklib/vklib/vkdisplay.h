#ifndef __VKLIB__VKDISPLAY_H_
#define __VKLIB__VKDISPLAY_H_

#include <stdbool.h>
#include <stdint.h>

#include "vk.h"

typedef struct
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkPresentModeKHR* present_modes;
    uint32_t present_mode_count;
    VkSurfaceFormatKHR* formats;
    uint32_t format_count;
} vklib_display_swapchain_info;

VKLIBAPI vklib_display_swapchain_info vklib_display_query_swapchain_info(VkSurfaceKHR surface, VkPhysicalDevice dev, bool fill);
VKLIBAPI void vklib_display_swapchain_info_dispose(vklib_display_swapchain_info* info);
VKLIBAPI bool vklib_display_create_swapchain(GLFWwindow* window, vk_data* vkd);
VKLIBAPI void vklib_display_dispose_swapchain(vk_data* vkd);

#endif
