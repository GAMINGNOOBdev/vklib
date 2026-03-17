#ifndef __VKLIB__VKRENDERER_H_
#define __VKLIB__VKRENDERER_H_

#include "vk.h"
#include "vklib/vkcmd.h"

#include <stdint.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

typedef struct
{
    VkSemaphore image_available, render_finished;
    VkFence frame_in_flight;
    uint32_t image;

    vklib_pipeline* pipeline;
    vklib_cmd* cmd;
} vklib_renderer;

VKLIBAPI vklib_renderer vklib_renderer_create(vklibd* vkd, vklib_pipeline* pipeline, vklib_cmd* cmd);
VKLIBAPI void vklib_renderer_begin(vklibd* vkd, vklib_renderer* renderer, VkClearValue clear_color);
VKLIBAPI void vklib_renderer_end(vklibd* vkd, vklib_renderer* renderer);
VKLIBAPI void vklib_renderer_destroy(vklibd* vkd, vklib_renderer* renderer);

#endif
