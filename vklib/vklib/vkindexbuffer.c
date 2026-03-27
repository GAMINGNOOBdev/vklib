#include "vklib/vk.h"
#include "vklib/vkbuffer.h"
#include "vklib/vkvertexbuffer.h"
#include "vklib/vkindexbuffer.h"

#include <memory.h>
#include <vulkan/vulkan_core.h>


vklib_index_buffer vklib_index_buffer_create(vklibd* vkd, vklib_cmd* cmd, const void* data, VkIndexType index_type, VkDeviceSize index_size, VkDeviceSize index_count)
{
    vklib_index_buffer ibo = {};
    assume(vkd && data && index_count && index_size, ibo);

    ibo.size = index_count * index_size;
    ibo.count = index_count;
    ibo.type = index_type;

    vklib_buffer staging_buffer = vklib_buffer_create(vkd, (vklib_buffer_create_info){
        .size = ibo.size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .memflags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    });
    vklib_buffer_fill_data(vkd, &staging_buffer, data, ibo.size);

    ibo.buffer = vklib_buffer_create(vkd, (vklib_buffer_create_info){
        .size = ibo.size,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        .memflags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    });

    vklib_buffer_copy(vkd, cmd, &ibo.buffer, &staging_buffer, ibo.size);

    vklib_buffer_destroy(vkd, &staging_buffer);

    return ibo;
}

void vklib_index_buffer_render(vklib_index_buffer* ibo, vklib_vertex_buffer* vbo, VkCommandBuffer cmd)
{
    assume(ibo);

    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, &vbo->buffer.buffer, offsets);
    vkCmdBindIndexBuffer(cmd, ibo->buffer.buffer, 0, ibo->type);

    vkCmdDrawIndexed(cmd, ibo->count, 1, 0, 0, 0);
}

void vklib_index_buffer_destroy(vklibd* vkd, vklib_index_buffer* ibo)
{
    assume(vkd && ibo);

    vklib_buffer_destroy(vkd, &ibo->buffer);
}
