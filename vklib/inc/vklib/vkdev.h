#ifndef __VKLIB__VKDEV_H_
#define __VKLIB__VKDEV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "vk.h"

typedef struct
{
    uint32_t graphics;
    bool graphics_present;
    uint32_t presentation;
    bool presentation_present;
} vklib_dev_queue_family;

VKLIBAPI bool vklib_dev_pick(vklibd* vkd);
VKLIBAPI bool vklib_dev_destroy(vklibd* vkd);
VKLIBAPI vklib_dev_queue_family vklib_dev_find_queue_families(VkSurfaceKHR surface, VkPhysicalDevice dev);

#ifdef __cplusplus
}
#endif

#endif
