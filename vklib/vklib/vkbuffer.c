#include "vklib/vk.h"
#include "vklib/vkbuffer.h"
#include <vulkan/vulkan_core.h>

#include <memory.h>

uint32_t vklib_buffer_find_mem_type(vklibd* vkd, uint32_t filter, VkMemoryPropertyFlags properties)
{
    assume(vkd, -1);

    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(vkd->physical_device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
    {
        if ((filter & (1 << i)) && ((memory_properties.memoryTypes[i].propertyFlags & properties) == properties))
            return i;
    }

    assume(false, -1);
}

vklib_buffer vklib_buffer_create(vklibd* vkd, vklib_buffer_create_info* info)
{
    vklib_buffer buffer = {};
    assume(vkd && vkd->instance, buffer);

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = info->size;
    buffer_info.usage = info->usage;
    buffer_info.sharingMode = info->mode;

    assume(vkCreateBuffer(vkd->device, &buffer_info, NULL, &buffer.buffer) == VK_SUCCESS, (vklib_buffer){});

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(vkd->device, buffer.buffer, &requirements);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = requirements.size;
    alloc_info.memoryTypeIndex = vklib_buffer_find_mem_type(vkd, requirements.memoryTypeBits, info->memflags);

    assume(vkAllocateMemory(vkd->device, &alloc_info, NULL, &buffer.memory) == VK_SUCCESS, (vklib_buffer){});

    vkBindBufferMemory(vkd->device, buffer.buffer, buffer.memory, 0);
    buffer.size = info->size;

    return buffer;
}

void vklib_buffer_fill_data(vklibd* vkd, vklib_buffer* buffer, const void* data, VkDeviceSize size)
{
    assume(vkd && buffer && data);

    if (size == (VkDeviceSize)-1 || size > (buffer->size))
        size = buffer->size;

    void* mem;
    vkMapMemory(vkd->device, buffer->memory, 0, buffer->size, 0, &mem);

    memcpy(mem, data, size);

    vkUnmapMemory(vkd->device, buffer->memory);
}

void vklib_buffer_destroy(vklibd* vkd, vklib_buffer* buffer)
{
    assume(vkd && buffer);

    vkDestroyBuffer(vkd->device, buffer->buffer, NULL);
    vkFreeMemory(vkd->device, buffer->memory, NULL);
}
