#ifndef __VKLIB__VKUNIFORM_H_
#define __VKLIB__VKUNIFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vk.h"
#include "vklib/vkbuffer.h"
#include "vklib/vkpipeline.h"
#include "vklib/vkrenderer.h"

#include <stdint.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

typedef struct
{
    size_t data_size;
    vklib_pipeline* pipeline;
    vklib_renderer* renderer;
} vklib_uniform_create_info;

typedef struct
{
    VkDescriptorSetLayout* layouts;
    VkDescriptorPool pool;
    VkDescriptorSet* sets;
    vklib_buffer* buffers;
    size_t data_size;
    void** memory;
    size_t count;
} vklib_uniform;

VKLIBAPI vklib_uniform vklib_uniform_create(vklibd* vkd, vklib_uniform_create_info info);
VKLIBAPI void vklib_uniform_update(vklib_uniform* uniform, size_t index, void* data, size_t size);
VKLIBAPI void vklib_uniform_bind(vklib_uniform* uniform, vklib_renderer* renderer);
VKLIBAPI void vklib_uniform_destroy(vklibd* vkd, vklib_uniform* uniform);

#ifdef __cplusplus
}
#endif

#endif
