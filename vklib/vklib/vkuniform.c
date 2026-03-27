#include "vklib/vk.h"
#include "vklib/vkbuffer.h"
#include "vklib/vkrenderer.h"
#include "vklib/vkuniform.h"
#include <vulkan/vulkan_core.h>

#include <stdlib.h>
#include <memory.h>

vklib_uniform vklib_uniform_create(vklibd* vkd, vklib_uniform_create_info info)
{
    vklib_uniform uniform = {};
    assume(vkd && info.data_size && info.pipeline && info.renderer, uniform);

    uniform.data_size = info.data_size;
    uniform.count = info.renderer->max_frames_in_flight;
    uniform.buffers = malloc(sizeof(vklib_buffer) * uniform.count);
    uniform.memory = malloc(sizeof(void*) * uniform.count);

    for (size_t i = 0; i < uniform.count; i++)
    {
        uniform.buffers[i] = vklib_buffer_create(vkd, (vklib_buffer_create_info){
            .size = uniform.data_size,
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            .memflags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        });
        vkMapMemory(vkd->device, uniform.buffers[i].memory, 0, uniform.data_size, 0, &uniform.memory[i]);
    }

    VkDescriptorPoolSize pool_size = {};
    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = uniform.count;

    VkDescriptorPoolCreateInfo pool_create_info = {};
    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.poolSizeCount = 1;
    pool_create_info.pPoolSizes = &pool_size;
    pool_create_info.maxSets = uniform.count;

    assume(vkCreateDescriptorPool(vkd->device, &pool_create_info, NULL, &uniform.pool) == VK_SUCCESS, (vklib_uniform){});

    uniform.layouts = malloc(sizeof(VkDescriptorSetLayout) * uniform.count);
    uniform.sets = malloc(sizeof(VkDescriptorSet) * uniform.count);
    for (size_t i = 0; i < uniform.count; i++)
        memcpy(&uniform.layouts[i], &info.pipeline->descriptor_set_layout, sizeof(VkDescriptorSetLayout));

    VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {};
    descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_alloc_info.descriptorPool = uniform.pool;
    descriptor_set_alloc_info.descriptorSetCount = uniform.count;
    descriptor_set_alloc_info.pSetLayouts = uniform.layouts;

    assume(vkAllocateDescriptorSets(vkd->device, &descriptor_set_alloc_info, uniform.sets) == VK_SUCCESS, (vklib_uniform){});

    for (size_t i = 0; i < uniform.count; i++)
    {
        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = uniform.buffers[i].buffer;
        buffer_info.offset = 0;
        buffer_info.range = uniform.data_size;

        VkWriteDescriptorSet descriptor_write = {};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = uniform.sets[i];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        descriptor_write.pBufferInfo = &buffer_info;

        vkUpdateDescriptorSets(vkd->device, 1, &descriptor_write, 0, NULL);
    }

    return uniform;
}

void vklib_uniform_update(vklib_uniform* uniform, size_t index, void* data, size_t size)
{
    assume(uniform && uniform->count && data && index <= uniform->count);

    if (size == 0 || size > uniform->data_size)
        size = uniform->data_size;

    memcpy(uniform->memory[index], data, size);
}

void vklib_uniform_bind(vklib_uniform* uniform, vklib_renderer* renderer)
{
    assume(uniform && uniform->count && renderer);

    VkCommandBuffer cmd = vklib_renderer_get_current_cmd_buffer(renderer);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline->layout, 0, 1, &uniform->sets[renderer->frame], 0, NULL);
}

void vklib_uniform_destroy(vklibd* vkd, vklib_uniform* uniform)
{
    assume(vkd && uniform && uniform->count);

    for (size_t i = 0; i < uniform->count; i++)
        vklib_buffer_destroy(vkd, &uniform->buffers[i]);

    free(uniform->buffers);
    free(uniform->memory);

    free(uniform->layouts);
    free(uniform->sets);

    vkDestroyDescriptorPool(vkd->device, uniform->pool, NULL);
}
