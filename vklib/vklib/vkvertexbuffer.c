#include "vklib/vk.h"
#include "vklib/vkbuffer.h"
#include "vklib/vkvertexbuffer.h"

#include <memory.h>
#include <vulkan/vulkan_core.h>


vklib_vertex_buffer vklib_vertex_buffer_create(vklibd* vkd, vklib_cmd* cmd, const void* data, VkDeviceSize vertex_size, VkDeviceSize vertex_count)
{
    vklib_vertex_buffer vbo = {};
    assume(vkd && data && vertex_count && vertex_size, vbo);

    vbo.size = vertex_count * vertex_size;
    vbo.count = vertex_count;

    vklib_buffer staging_buffer = vklib_buffer_create(vkd, (vklib_buffer_create_info){
        .size = vbo.size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .memflags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    });
    vklib_buffer_fill_data(vkd, &staging_buffer, data, vbo.size);

    vbo.buffer = vklib_buffer_create(vkd, (vklib_buffer_create_info){
        .size = vbo.size,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .memflags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    });

    vklib_buffer_copy(vkd, cmd, &vbo.buffer, &staging_buffer, vbo.size);

    vklib_buffer_destroy(vkd, &staging_buffer);

    return vbo;
}

void vklib_vertex_buffer_render(vklib_vertex_buffer* vbo, VkCommandBuffer cmd)
{
    assume(vbo);

    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, &vbo->buffer.buffer, offsets);

    vkCmdDraw(cmd, vbo->count, 1, 0, 0);
}

void vklib_vertex_buffer_destroy(vklibd* vkd, vklib_vertex_buffer* vbo)
{
    assume(vkd && vbo);

    vklib_buffer_destroy(vkd, &vbo->buffer);
}
