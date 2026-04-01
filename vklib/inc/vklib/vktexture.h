#ifndef __VKLIB__VKTEXTURE_H_
#define __VKLIB__VKTEXTURE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vk.h"
#include "vkcmd.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

typedef struct
{
    VkImage image;
    VkFormat format;
    VkExtent3D extent;
    VkDeviceMemory memory;
} vklib_image;

typedef struct
{
    void* data;
    size_t data_size;
    VkExtent3D size;
    VkSampler sampler;

    VkFormat format;
    uint32_t miplevel;
    VkImageTiling tiling;
} vklib_texture_create_info;

typedef struct
{
    uint32_t miplevel;
    VkSampler sampler;
    vklib_image image;
    VkImageView image_view;
} vklib_texture;

VKLIBAPI vklib_image vklib_image_create(vklibd* vkd, VkExtent3D size, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
VKLIBAPI void vklib_image_destroy(vklibd* vkd, vklib_image* image);

VKLIBAPI VkImageView vklib_image_view_create(vklibd* vkd, vklib_image* image, VkImageAspectFlagBits aspect);
VKLIBAPI void vklib_image_view_destroy(vklibd* vkd, VkImageView image_view);

VKLIBAPI VkSampler vklib_texture_sampler_create(vklibd* vkd, VkSamplerCreateInfo info);
VKLIBAPI void vklib_texture_sampler_destroy(vklibd* vkd, VkSampler sampler);

VKLIBAPI vklib_texture vklib_texture_create(vklibd* vkd, vklib_cmd* cmd, vklib_texture_create_info info);
VKLIBAPI void vklib_texture_destroy(vklibd* vkd, vklib_texture* texture);

#ifdef __cplusplus
}
#endif

#endif
