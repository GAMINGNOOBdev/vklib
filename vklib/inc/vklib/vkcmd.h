#ifndef __VKLIB__VKCMD_H_
#define __VKLIB__VKCMD_H_

#include "vk.h"
#include "vkpipeline.h"

#include <stdint.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

typedef struct
{
    VkCommandPool pool;
    VkCommandBuffer buffer;
} vklib_cmd;

VKLIBAPI vklib_cmd vklib_cmd_create(vklibd* vkd);
VKLIBAPI bool vklib_cmd_begin(vklib_cmd* cmd, vklib_pipeline* pipeline);
VKLIBAPI void vklib_cmd_end(vklib_cmd* cmd);
VKLIBAPI void vklib_cmd_destroy(vklibd* vkd, vklib_cmd* cmd);

#endif
