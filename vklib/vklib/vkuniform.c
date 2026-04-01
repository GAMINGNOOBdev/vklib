#include "vklib/vk.h"
#include "vklib/vkbuffer.h"
#include "vklib/vkrenderer.h"
#include "vklib/vkuniform.h"

#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <vulkan/vulkan_core.h>

vklib_uniform_descriptor vklib_uniform_descriptor_create(vklibd* vkd, vklib_uniform_descriptor_create_info info)
{
    vklib_uniform_descriptor descriptor = {};
    assume(vkd, descriptor);

    descriptor.size = info.size;
    descriptor.type = info.type;
    if (descriptor.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
    {
        descriptor.buffer = vklib_buffer_create(vkd, (vklib_buffer_create_info){
            .size = info.size,
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            .memflags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        });
        vkMapMemory(vkd->device, descriptor.buffer.memory, 0, descriptor.size, 0, &descriptor.memory);

        return descriptor;
    }
    else if (descriptor.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
    {
        descriptor.image = info.image;
        return descriptor;
    }

    LOGERROR("unsupported uniform descriptor type");
    return descriptor;
}

void vklib_uniform_descriptor_get_write_set_info(vklib_uniform_descriptor* descriptor, VkWriteDescriptorSet* set, VkDescriptorSet destination_set, uint32_t binding)
{
    assume(descriptor && set && destination_set != VK_NULL_HANDLE);

    VkWriteDescriptorSet write_set = {};
    write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_set.dstSet = destination_set;
    write_set.dstBinding = binding;
    write_set.dstArrayElement = 0;
    write_set.descriptorType = descriptor->type;
    write_set.descriptorCount = 1;

    if (descriptor->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
    {
        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = descriptor->buffer.buffer;
        buffer_info.offset = 0;
        buffer_info.range = descriptor->size;

        write_set.pBufferInfo = malloc(sizeof(VkDescriptorBufferInfo));
        memcpy((void*)write_set.pBufferInfo, &buffer_info, sizeof(VkDescriptorBufferInfo));
    }
    else if (descriptor->type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
    {
        VkDescriptorImageInfo image_info = {};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = descriptor->image->image_view;
        image_info.sampler = descriptor->image->sampler;

        write_set.pImageInfo = malloc(sizeof(VkDescriptorImageInfo));
        memcpy((void*)write_set.pImageInfo, &image_info, sizeof(VkDescriptorImageInfo));
    }
    else
    {
        LOGERROR("unsupported uniform descriptor type");
        return;
    }

    *set = write_set;
}

void vklib_uniform_descriptor_clear_write_set_info(vklib_uniform_descriptor* descriptor, VkWriteDescriptorSet* set)
{
    assume(descriptor && set);

    if (descriptor->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        free((void*)set->pBufferInfo);
    else if (descriptor->type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
        free((void*)set->pImageInfo);
    else
    {
        LOGERROR("unsupported uniform descriptor type");
        return;
    }
}

void vklib_uniform_descriptor_destroy(vklibd* vkd, vklib_uniform_descriptor* descriptor)
{
    assume(vkd && descriptor);

    if (descriptor->type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        return;

    vklib_buffer_destroy(vkd, &descriptor->buffer);
}

vklib_uniform_entry vklib_uniform_entry_create(vklibd* vkd, vklib_uniform_descriptor_create_info* descriptor_info, uint32_t descriptor_count)
{
    vklib_uniform_entry entry = {};
    assume(vkd && descriptor_info && descriptor_count, entry);

    entry.descriptor_count = descriptor_count;
    entry.descriptors = malloc(sizeof(vklib_uniform_descriptor) * descriptor_count);
    assume(entry.descriptors, (vklib_uniform_entry){});

    for (uint32_t i = 0; i < descriptor_count; i++)
        entry.descriptors[i] = vklib_uniform_descriptor_create(vkd, descriptor_info[i]);

    return entry;
}

void vklib_uniform_entry_get_descriptor_write_sets(vklib_uniform_entry* entry, VkWriteDescriptorSet* sets, VkDescriptorSet destination_set)
{
    assume(entry && sets && destination_set != VK_NULL_HANDLE);

    for (uint32_t i = 0; i < entry->descriptor_count; i++)
        vklib_uniform_descriptor_get_write_set_info(&entry->descriptors[i], &sets[i], destination_set, i);
}

void vklib_uniform_entry_clear_descriptor_write_sets(vklib_uniform_entry* entry, VkWriteDescriptorSet* sets)
{
    assume(entry && sets);

    for (uint32_t i = 0; i < entry->descriptor_count; i++)
        vklib_uniform_descriptor_clear_write_set_info(&entry->descriptors[i], &sets[i]);
}

void vklib_uniform_entry_destroy(vklibd* vkd, vklib_uniform_entry* entry)
{
    assume(vkd && entry);

    for (uint32_t i = 0; i < entry->descriptor_count; i++)
        vklib_uniform_descriptor_destroy(vkd, &entry->descriptors[i]);

    free(entry->descriptors);
}

vklib_uniform vklib_uniform_create(vklibd* vkd, vklib_uniform_create_info info)
{
    vklib_uniform uniform = {};
    assume(vkd && info.descriptor_count && info.pipeline && info.renderer, uniform);

    uniform.count = info.renderer->max_frames_in_flight;
    uniform.entries = malloc(sizeof(vklib_uniform_entry) * uniform.count);

    for (size_t i = 0; i < uniform.count; i++)
        uniform.entries[i] = vklib_uniform_entry_create(vkd, info.descriptors, info.descriptor_count);

    VkDescriptorPoolSize* pool_sizes = malloc(sizeof(VkDescriptorPoolSize) * info.descriptor_count);
    for (uint32_t i = 0; i < info.descriptor_count; i++)
    {
        pool_sizes[i].type = info.descriptors[i].type;
        pool_sizes[i].descriptorCount = uniform.count;
    }

    VkDescriptorPoolCreateInfo pool_create_info = {};
    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.poolSizeCount = info.descriptor_count;
    pool_create_info.pPoolSizes = pool_sizes;
    pool_create_info.maxSets = uniform.count;

    assume(vkCreateDescriptorPool(vkd->device, &pool_create_info, NULL, &uniform.pool) == VK_SUCCESS, (vklib_uniform){});
    free(pool_sizes);

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

    VkWriteDescriptorSet* write_sets = malloc(sizeof(VkWriteDescriptorSet) * info.descriptor_count);
    for (size_t i = 0; i < uniform.count; i++)
    {
        vklib_uniform_entry_get_descriptor_write_sets(&uniform.entries[i], write_sets, uniform.sets[i]);

        vkUpdateDescriptorSets(vkd->device, info.descriptor_count, write_sets, 0, NULL);

        vklib_uniform_entry_clear_descriptor_write_sets(&uniform.entries[i], write_sets);
    }
    free(write_sets);

    return uniform;
}

void vklib_uniform_update_descriptor(vklib_uniform* uniform, size_t index, uint32_t descriptor_binding, void* data, size_t size)
{
    assume(uniform && uniform->count && data && index <= uniform->count);

    vklib_uniform_entry entry = uniform->entries[index];
    assume(descriptor_binding < entry.descriptor_count);

    vklib_uniform_descriptor descriptor = entry.descriptors[descriptor_binding];

    if (size == 0 || size > descriptor.size)
        size = descriptor.size;

    memcpy(descriptor.memory, data, size);
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

    for (uint32_t i = 0; i < uniform->count; i++)
        vklib_uniform_entry_destroy(vkd, &uniform->entries[i]);
    free(uniform->entries);

    free(uniform->layouts);
    free(uniform->sets);

    vkDestroyDescriptorPool(vkd->device, uniform->pool, NULL);
}
