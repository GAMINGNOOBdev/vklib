#ifndef __VKLIB__VKINDEXBUFFER_H_
#define __VKLIB__VKINDEXBUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vk.h"
#include "vkcmd.h"
#include "vkbuffer.h"
#include "vkvertexbuffer.h"

#include <stdint.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

typedef struct
{
    vklib_buffer buffer;
    VkIndexType type;
    size_t count;
    size_t size;
} vklib_index_buffer;

VKLIBAPI vklib_index_buffer vklib_index_buffer_create(vklibd* vkd, vklib_cmd* cmd, const void* data, VkIndexType index_type, VkDeviceSize index_size, VkDeviceSize index_count);
VKLIBAPI void vklib_index_buffer_render(vklib_index_buffer* ibo, vklib_vertex_buffer* vbo, VkCommandBuffer cmd);
VKLIBAPI void vklib_index_buffer_destroy(vklibd* vkd, vklib_index_buffer* ibo);

#ifdef __cplusplus
}
#endif

#endif
