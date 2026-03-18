#include "vklib/vk.h"
#include "vklib/vkcmd.h"
#include "vklib/vkdev.h"

#include <stdlib.h>
#include <memory.h>
#include <vulkan/vulkan_core.h>

vklib_cmd vklib_cmd_create(vklibd* vkd, uint32_t buffer_count)
{
    vklib_cmd cmd = {};
    assume(vkd && vkd->instance, cmd);

    cmd.buffer_count = buffer_count;
    
    vklib_dev_queue_family family = vklib_dev_find_queue_families(vkd->surface, vkd->physical_device);

    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = family.graphics;

    if (vkCreateCommandPool(vkd->device, &pool_info, NULL, &cmd.pool) != VK_SUCCESS)
    {
        LOGERROR("unable to create command pool");
        return (vklib_cmd){};
    }

    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = cmd.pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = cmd.buffer_count;

    cmd.buffers = malloc(sizeof(VkCommandBuffer)*cmd.buffer_count);

    if (vkAllocateCommandBuffers(vkd->device, &alloc_info, cmd.buffers) != VK_SUCCESS)
    {
        free(cmd.buffers);
        LOGERROR("unable to allocate command buffer");
        return (vklib_cmd){};
    }

    return cmd;
}

bool vklib_cmd_begin(vklib_cmd* cmd, vklib_pipeline* pipeline, uint32_t idx)
{
    assume(cmd && pipeline, false);

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = NULL;

    return vkBeginCommandBuffer(cmd->buffers[idx], &begin_info) == VK_SUCCESS;
}

void vklib_cmd_end(vklib_cmd* cmd, uint32_t idx)
{
    assume(cmd);

    vkEndCommandBuffer(cmd->buffers[idx]);
}

void vklib_cmd_destroy(vklibd* vkd, vklib_cmd* cmd)
{
    assume(vkd && cmd);

    free(cmd->buffers);

    vkDestroyCommandPool(vkd->device, cmd->pool, NULL);
}
