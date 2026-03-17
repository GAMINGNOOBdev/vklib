#ifndef __VKLIB__VKFRAMEBUFFER_H_
#define __VKLIB__VKFRAMEBUFFER_H_

#include "vk.h"
#include "vkpipeline.h"

#include <stdint.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

VKLIBAPI void vklib_framebuffer_create(vklibd* vkd, VkRenderPass render_pass, int n, VkFramebuffer* buffers, VkImageView* image_views);
VKLIBAPI void vklib_framebuffer_destroy(vklibd* vkd, int n, VkFramebuffer* buffers);

#endif
