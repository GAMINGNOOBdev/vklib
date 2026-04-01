#ifndef __VKLIB__VKRENDERTARGET_H_
#define __VKLIB__VKRENDERTARGET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vk.h"
#include "vktexture.h"

#include <stdint.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

typedef struct
{
    VkRenderPass render_pass;
    uint32_t max_frame_count;
    VkExtent2D extent;
    VkFormat format;
} vklib_render_target_create_info;

typedef struct
{
    VkFramebuffer* framebuffers;
    VkRenderPass render_pass;
    uint32_t frame_count;
    VkExtent2D extent;
    VkFormat format;

    vklib_texture* textures;
    uint32_t texture_count;
} vklib_render_target;

VKLIBAPI VkRenderPass vklib_render_pass_create(vklibd* vkd);
VKLIBAPI void vklib_render_pass_destroy(vklibd* vkd, VkRenderPass render_pass);

VKLIBAPI vklib_render_target vklib_render_target_create(vklibd* vkd, vklib_render_target_create_info info);
VKLIBAPI void vklib_render_target_recreate(vklibd* vkd, vklib_render_target* target);
VKLIBAPI void vklib_render_target_destroy(vklibd* vkd, vklib_render_target* target);

#ifdef __cplusplus
}
#endif

#endif