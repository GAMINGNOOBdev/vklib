#ifndef __VKLIB__VKCMD_H_
#define __VKLIB__VKCMD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vk.h"

#include <stdint.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

typedef struct
{
    VkCommandPool pool;
    uint32_t buffer_count;
    VkCommandBuffer* buffers;
} vklib_cmd;

VKLIBAPI VkCommandBuffer vklib_cmd_begin_single_use(vklibd* vkd, vklib_cmd* cmd);
VKLIBAPI void vklib_cmd_end_single_use(vklibd* vkd, vklib_cmd* cmd, VkCommandBuffer buffer);

VKLIBAPI vklib_cmd vklib_cmd_create(vklibd* vkd, uint32_t buffer_count);
VKLIBAPI bool vklib_cmd_begin(vklib_cmd* cmd, uint32_t idx);
VKLIBAPI void vklib_cmd_end(vklib_cmd* cmd, uint32_t idx);
VKLIBAPI void vklib_cmd_destroy(vklibd* vkd, vklib_cmd* cmd);

#ifdef __cplusplus
}
#endif

#endif
