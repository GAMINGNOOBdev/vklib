#ifndef __VKLIB__VKPIPELINE_H_
#define __VKLIB__VKPIPELINE_H_

#include "vk.h"

#include <stdint.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

typedef struct
{
    VkShaderModule vertex, fragment;
    VkPrimitiveTopology draw_mode;
    VkRenderPass render_pass;
    VkPipelineLayout layout;
    VkPipeline handle;
} vklib_pipeline;

VKLIBAPI VkShaderModule vklib_pipeline_shader_module_create(vklibd* vkd, void* data, size_t size);
VKLIBAPI void vklib_pipeline_shader_module_destroy(vklibd* vkd, VkShaderModule module);

VKLIBAPI VkRenderPass vklib_pipeline_render_pass_create(vklibd* vkd);
VKLIBAPI void vklib_pipeline_render_pass_destroy(vklibd* vkd, VkRenderPass render_pass);

VKLIBAPI vklib_pipeline vklib_pipeline_create(vklibd* vkd, VkShaderModule vertex, VkShaderModule fragment, VkPrimitiveTopology draw_mode, bool wireframe);
VKLIBAPI void vklib_pipeline_destroy(vklibd* vkd, vklib_pipeline* pipeline);

#endif
