#ifndef __VKLIB__VKDEV_H_
#define __VKLIB__VKDEV_H_

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

VKLIBAPI bool vklib_dev_pick(vk_data* vkd);
VKLIBAPI bool vklib_dev_dispose(vk_data* vkd);
VKLIBAPI vklib_dev_queue_family vklib_dev_find_queue_families(VkSurfaceKHR surface, VkPhysicalDevice dev);

#endif
