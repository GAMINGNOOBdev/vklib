#ifndef __VKLIB__VKUTIL_H_
#define __VKLIB__VKUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "vk.h"

VKLIBAPI bool vklib_util_check_validation_layer_support(void);
VKLIBAPI const char** vklib_util_get_enabled_layers(uint32_t* count);
VKLIBAPI const char** vklib_util_get_required_extensions(uint32_t* count);
VKLIBAPI bool vklib_util_debugging_enabled(void);

VKLIBAPI bool vklib_util_init_debug_messenger(vklibd* data);
VKLIBAPI void vklib_util_destroy_debug_messenger(vklibd* data);

VKLIBAPI VkSurfaceFormatKHR vklib_util_choose_swap_surface_format(VkSurfaceFormatKHR* formats, uint32_t count);
VKLIBAPI VkPresentModeKHR vklib_util_choose_swap_present_mode(VkPresentModeKHR* modes, uint32_t count);
VKLIBAPI VkExtent2D vklib_util_choose_swap_extent(GLFWwindow* window, VkSurfaceCapabilitiesKHR capabilities);

#ifdef __cplusplus
}
#endif

#endif
