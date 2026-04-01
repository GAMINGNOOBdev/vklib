#ifndef __VKLIB__VKPIPELINE_H_
#define __VKLIB__VKPIPELINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vk.h"

#include <stdint.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

typedef VkVertexInputAttributeDescription vklib_pipeline_vertex_attribute_info;

typedef struct
{
    VkShaderModule vertex;
    VkShaderModule fragment;
    VkPrimitiveTopology draw_mode;
    bool wireframe;

    VkRenderPass render_pass;

    vklib_pipeline_vertex_attribute_info* vertex_attrib_info;
    uint32_t vertex_attrib_info_count;
    size_t vertex_size;

    VkDescriptorSetLayoutBinding* uniform_bindings;
    size_t uniform_binding_count;
} vklib_pipeline_create_info;

typedef struct
{
    VkDescriptorSetLayout descriptor_set_layout;
    VkShaderModule vertex, fragment;
    VkPrimitiveTopology draw_mode;
    VkRenderPass render_pass;
    VkPipelineLayout layout;
    VkPipeline handle;
} vklib_pipeline;

VKLIBAPI VkShaderModule vklib_pipeline_shader_module_create(vklibd* vkd, void* data, size_t size);
VKLIBAPI void vklib_pipeline_shader_module_destroy(vklibd* vkd, VkShaderModule module);

VKLIBAPI vklib_pipeline vklib_pipeline_create(vklibd* vkd, vklib_pipeline_create_info create_info);
VKLIBAPI void vklib_pipeline_destroy(vklibd* vkd, vklib_pipeline* pipeline);

#ifdef __cplusplus
}
#endif

#endif
