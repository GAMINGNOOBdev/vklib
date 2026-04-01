#ifndef __VKLIB__VKRENDERER_H_
#define __VKLIB__VKRENDERER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vk.h"
#include "vklib/vkcmd.h"
#include "vklib/vkpipeline.h"
#include "vklib/vkrendertarget.h"

#include <stdint.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

typedef struct
{
    VkSemaphore* images_available;
    VkSemaphore* renders_finished;
    VkFence* frames_in_flight;
    uint32_t max_frames_in_flight;
    uint32_t frame;
    uint32_t image;

    vklib_pipeline* pipeline;
    vklib_cmd cmd;
} vklib_renderer;

VKLIBAPI vklib_renderer vklib_renderer_create(vklibd* vkd, vklib_pipeline* pipeline, uint32_t max_frame_in_flight);
VKLIBAPI VkCommandBuffer vklib_renderer_get_current_cmd_buffer(vklib_renderer* renderer);
VKLIBAPI bool vklib_renderer_begin(vklibd* vkd, vklib_renderer* renderer, vklib_render_target* render_target, VkClearValue clear_color);
VKLIBAPI bool vklib_renderer_end(vklibd* vkd, vklib_renderer* renderer, vklib_render_target* render_target);
VKLIBAPI void vklib_renderer_destroy(vklibd* vkd, vklib_renderer* renderer);

#ifdef __cplusplus
}
#endif

#endif
