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

vklib_buffer vklib_buffer_create(vklibd* vkd, vklib_buffer_create_info info)
{
    vklib_buffer buffer = {};
    assume(vkd && vkd->instance, buffer);

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = info.size;
    buffer_info.usage = info.usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    assume(vkCreateBuffer(vkd->device, &buffer_info, NULL, &buffer.buffer) == VK_SUCCESS, (vklib_buffer){});

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(vkd->device, buffer.buffer, &requirements);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = requirements.size;
    alloc_info.memoryTypeIndex = vklib_buffer_find_mem_type(vkd, requirements.memoryTypeBits, info.memflags);

    assume(vkAllocateMemory(vkd->device, &alloc_info, NULL, &buffer.memory) == VK_SUCCESS, (vklib_buffer){});

    vkBindBufferMemory(vkd->device, buffer.buffer, buffer.memory, 0);
    buffer.size = info.size;

    return buffer;
}

void vklib_buffer_fill_data(vklibd* vkd, vklib_buffer* buffer, const void* data, VkDeviceSize size)
{
    assume(vkd && buffer && data);

    if (size == (VkDeviceSize)-1 || size > (buffer->size) || size == 0)
        size = buffer->size;

    void* mem;
    vkMapMemory(vkd->device, buffer->memory, 0, buffer->size, 0, &mem);

    memcpy(mem, data, size);

    vkUnmapMemory(vkd->device, buffer->memory);
}

void vklib_buffer_copy(vklibd* vkd, vklib_cmd* cmd, vklib_buffer* dst, vklib_buffer* src, VkDeviceSize size)
{
    assume(vkd && cmd && dst && src);
    VkDeviceSize max_size = dst->size > src->size ? src->size : dst->size;
    if (size == (VkDeviceSize)-1 || size > max_size || size == 0)
        size = max_size;

    VkCommandBuffer cmdbuf = vklib_cmd_begin_single_use(vkd, cmd);

    VkBufferCopy copy_info = {};
    copy_info.size = size;
    vkCmdCopyBuffer(cmdbuf, src->buffer, dst->buffer, 1, &copy_info);

    vklib_cmd_end_single_use(vkd, cmd, cmdbuf);
}

void vklib_buffer_copy_to_image(vklibd* vkd, vklib_cmd* cmd, vklib_buffer* src, VkImage dst, VkExtent3D size, VkImageAspectFlags aspect)
{
    assume(vkd && cmd && src && dst != VK_NULL_HANDLE);

    VkCommandBuffer cmdbuf = vklib_cmd_begin_single_use(vkd, cmd);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = aspect;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = (VkOffset3D){0,0,0};
    region.imageExtent = size;

    vkCmdCopyBufferToImage(cmdbuf, src->buffer, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    vklib_cmd_end_single_use(vkd, cmd, cmdbuf);
}

void vklib_buffer_destroy(vklibd* vkd, vklib_buffer* buffer)
{
    assume(vkd && buffer);

    vkDestroyBuffer(vkd->device, buffer->buffer, NULL);
    vkFreeMemory(vkd->device, buffer->memory, NULL);
}
