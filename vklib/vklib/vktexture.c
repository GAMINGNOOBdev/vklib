#include "vklib/vk.h"
#include "vklib/vkbuffer.h"
#include "vklib/vktexture.h"
#include <vulkan/vulkan_core.h>

#include <memory.h>

vklib_image vklib_image_create(vklibd* vkd, VkExtent3D size, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
{
    vklib_image image = {};
    assume(vkd, image);

    image.extent = size;
    image.format = format;

    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent = size;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    assume(vkCreateImage(vkd->device, &image_info, NULL, &image.image) == VK_SUCCESS, (vklib_image){});

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(vkd->device, image.image, &requirements);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = requirements.size;
    alloc_info.memoryTypeIndex = vklib_buffer_find_mem_type(vkd, requirements.memoryTypeBits, properties);

    assume(vkAllocateMemory(vkd->device, &alloc_info, NULL, &image.memory) == VK_SUCCESS, (vklib_image){});

    vkBindImageMemory(vkd->device, image.image, image.memory, 0);
    return image;
}

void vklib_image_destroy(vklibd* vkd, vklib_image* image)
{
    assume(vkd && image);

    vkDestroyImage(vkd->device, image->image, NULL);
    vkFreeMemory(vkd->device, image->memory, NULL);
}

VkImageView vklib_image_view_create(vklibd* vkd, vklib_image* image, VkImageAspectFlagBits aspect)
{
    assume(vkd && image, VK_NULL_HANDLE);

    VkImageView image_view = VK_NULL_HANDLE;

    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image->image;
    view_info.viewType = image->extent.depth == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_3D;
    view_info.format = image->format;
    view_info.subresourceRange.aspectMask = aspect;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    assume(vkCreateImageView(vkd->device, &view_info, NULL, &image_view) == VK_SUCCESS, VK_NULL_HANDLE);

    return image_view;
}

void vklib_image_view_destroy(vklibd* vkd, VkImageView image_view)
{
    assume(vkd && image_view != VK_NULL_HANDLE);

    vkDestroyImageView(vkd->device, image_view, NULL);
}

VkSampler vklib_texture_sampler_create(vklibd* vkd, VkSamplerCreateInfo info)
{
    assume(vkd, VK_NULL_HANDLE);

    VkSampler sampler;

    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(vkd->physical_device, &properties);

    if (info.maxAnisotropy > properties.limits.maxSamplerAnisotropy)
        info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    assume(vkCreateSampler(vkd->device, &info, NULL, &sampler) == VK_SUCCESS, VK_NULL_HANDLE);
    return sampler;
}

void vklib_texture_sampler_destroy(vklibd* vkd, VkSampler sampler)
{
    assume(vkd && sampler != VK_NULL_HANDLE);
    vkDestroySampler(vkd->device, sampler, NULL);
}

void vklib_texture_transition_layout(vklibd* vkd, vklib_cmd* cmd, VkImage img, VkImageAspectFlags aspect, uint32_t miplevel, VkFormat fmt, VkImageLayout old, VkImageLayout new)
{
    assume(vkd && cmd && img != VK_NULL_HANDLE);

    VkCommandBuffer cmdbuf = vklib_cmd_begin_single_use(vkd, cmd);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old;
    barrier.newLayout = new;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = img;
    barrier.subresourceRange.aspectMask = aspect;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = miplevel;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags src_stage;
    VkPipelineStageFlags dst_stage;

    if (old == VK_IMAGE_LAYOUT_UNDEFINED && new == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(
        cmdbuf,
        src_stage, dst_stage,
        0,
        0, NULL,
        0, NULL,
        1, &barrier
    );

    vklib_cmd_end_single_use(vkd, cmd, cmdbuf);
}

vklib_texture vklib_texture_create(vklibd* vkd, vklib_cmd* cmd, vklib_texture_create_info info)
{
    vklib_texture texture = {};
    assume(vkd && cmd && info.data && info.data_size, texture);

    VkImageAspectFlags img_aspect = VK_IMAGE_ASPECT_COLOR_BIT;

    // base data creation and copying to the gpu

    vklib_buffer staging_buffer = vklib_buffer_create(vkd, (vklib_buffer_create_info){
        info.data_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    });
    vklib_buffer_fill_data(vkd, &staging_buffer, info.data, 0);

    texture.image = vklib_image_create(vkd, info.size, info.format, info.tiling, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    texture.miplevel = info.miplevel;

    vklib_texture_transition_layout(vkd, cmd, texture.image.image, img_aspect, info.miplevel, info.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vklib_buffer_copy_to_image(vkd, cmd, &staging_buffer, texture.image.image, texture.image.extent, img_aspect);
    vklib_texture_transition_layout(vkd, cmd, texture.image.image, img_aspect, info.miplevel, info.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vklib_buffer_destroy(vkd, &staging_buffer);

    // texture image view creation

    texture.image_view = vklib_image_view_create(vkd, &texture.image, VK_IMAGE_ASPECT_COLOR_BIT);

    texture.sampler = info.sampler;

    return texture;
}

void vklib_texture_destroy(vklibd* vkd, vklib_texture* texture)
{
    assume(vkd && texture);

    vklib_image_view_destroy(vkd, texture->image_view);
    vklib_image_destroy(vkd, &texture->image);
}
