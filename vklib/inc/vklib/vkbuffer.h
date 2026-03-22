#ifndef __VKLIB__VKBUFFER_H_
#define __VKLIB__VKBUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vk.h"

#include <stdint.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

typedef struct
{
    VkDeviceSize size;
    VkSharingMode mode;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags memflags;
} vklib_buffer_create_info;

typedef struct
{
    VkBuffer buffer;
    VkDeviceSize size;
    VkDeviceMemory memory;
} vklib_buffer;

VKLIBAPI vklib_buffer vklib_buffer_create(vklibd* vkd, vklib_buffer_create_info* info);
VKLIBAPI void vklib_buffer_fill_data(vklibd* vkd, vklib_buffer* buffer, const void* data, VkDeviceSize size);
VKLIBAPI void vklib_buffer_destroy(vklibd* vkd, vklib_buffer* buffer);

#ifdef __cplusplus
}
#endif

#endif
