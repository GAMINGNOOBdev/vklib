#ifndef __VKLIB__VKUNIFORM_H_
#define __VKLIB__VKUNIFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vk.h"
#include "vklib/vkbuffer.h"
#include "vklib/vktexture.h"
#include "vklib/vkpipeline.h"
#include "vklib/vkrenderer.h"

#include <stdint.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

typedef struct
{
    size_t size;
    vklib_texture* image;
    VkDescriptorType type;
} vklib_uniform_descriptor_create_info;

typedef struct
{
    size_t size;
    void* memory;
    vklib_buffer buffer;
    vklib_texture* image;
    VkDescriptorType type;
} vklib_uniform_descriptor;

typedef struct
{
    vklib_uniform_descriptor_create_info* descriptors;
    uint32_t descriptor_count;
    vklib_pipeline* pipeline;
    vklib_renderer* renderer;
} vklib_uniform_create_info;

typedef struct
{
    vklib_uniform_descriptor* descriptors;
    uint32_t descriptor_count;
} vklib_uniform_entry;

typedef struct
{
    VkDescriptorSetLayout* layouts;
    vklib_uniform_entry* entries;
    VkDescriptorPool pool;
    VkDescriptorSet* sets;
    size_t count;
} vklib_uniform;

VKLIBAPI vklib_uniform_entry vklib_uniform_entry_create(vklibd* vkd, vklib_uniform_descriptor_create_info* descriptor_info, uint32_t descriptor_count);
VKLIBAPI void vklib_uniform_entry_destroy(vklibd* vkd, vklib_uniform_entry* entry);

VKLIBAPI vklib_uniform vklib_uniform_create(vklibd* vkd, vklib_uniform_create_info info);
VKLIBAPI void vklib_uniform_update_descriptor(vklib_uniform* uniform, size_t index, uint32_t descriptor_binding, void* data, size_t size);
VKLIBAPI void vklib_uniform_bind(vklib_uniform* uniform, vklib_renderer* renderer);
VKLIBAPI void vklib_uniform_destroy(vklibd* vkd, vklib_uniform* uniform);

#ifdef __cplusplus
}
#endif

#endif
