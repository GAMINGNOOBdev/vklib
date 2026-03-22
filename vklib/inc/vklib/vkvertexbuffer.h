#ifndef __VKLIB__VKVERTEXBUFFER_H_
#define __VKLIB__VKVERTEXBUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vk.h"
#include "vkcmd.h"
#include "vkbuffer.h"

#include <stdint.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

typedef struct
{
    vklib_buffer buffer;
    size_t count;
    size_t size;
} vklib_vertex_buffer;

VKLIBAPI vklib_vertex_buffer vklib_vertex_buffer_create(vklibd* vkd, vklib_cmd* cmd, const void* data, VkDeviceSize vertex_size, VkDeviceSize vertex_count);
VKLIBAPI void vklib_vertex_buffer_render(vklib_vertex_buffer* buffer, VkCommandBuffer cmd);
VKLIBAPI void vklib_vertex_buffer_destroy(vklibd* vkd, vklib_vertex_buffer* buffer);

#ifdef __cplusplus
}
#endif

#endif
