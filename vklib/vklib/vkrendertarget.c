#include "vklib/vkrendertarget.h"
#include "vklib/vkframebuffer.h"
#include "vklib/vktexture.h"

#include <stdlib.h>
#include <memory.h>
#include <vulkan/vulkan_core.h>

VkRenderPass vklib_render_pass_create(vklibd* vkd)
{
    assume(vkd, VK_NULL_HANDLE);
    assume(vkd->instance, VK_NULL_HANDLE);

    VkAttachmentDescription color_attachment = {};
    color_attachment.format = vkd->swapchain_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_reference = {};
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    VkRenderPass render_pass = VK_NULL_HANDLE;
    if (vkCreateRenderPass(vkd->device, &render_pass_info, NULL, &render_pass) != VK_SUCCESS)
    {
        LOGERROR("failed to create render pass");
        return VK_NULL_HANDLE;
    }
    return render_pass;
}

void vklib_render_pass_destroy(vklibd* vkd, VkRenderPass render_pass)
{
    assume(vkd && vkd->instance && render_pass != VK_NULL_HANDLE);
    vkDestroyRenderPass(vkd->device, render_pass, NULL);
}

vklib_render_target vklib_render_target_create(vklibd* vkd, vklib_render_target_create_info info)
{
    vklib_render_target target = {};
    assume(vkd, target);

    target.frame_count = info.max_frame_count;
    target.render_pass = info.render_pass;
    target.extent = info.extent;
    target.format = info.format;

    target.framebuffers = malloc(sizeof(VkFramebuffer) * target.frame_count);

    // swapchain render target
    if (info.extent.width == 0 || info.extent.height == 0 || info.format == VK_FORMAT_UNDEFINED)
    {
        vklib_framebuffer_create(vkd, target.render_pass, target.frame_count, target.framebuffers, vkd->swapchain_image_views);
        target.extent = vkd->swapchain_extent;
        target.format = vkd->swapchain_format;
        return target;
    }

    target.texture_count = target.frame_count;
    target.textures = malloc(sizeof(vklib_texture) * target.frame_count);

    for (uint32_t i = 0; i < target.texture_count; i++)
    {
        target.textures[i].image = vklib_image_create(vkd, (VkExtent3D){info.extent.width, info.extent.height, 1}, info.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        target.textures[i].image_view = vklib_image_view_create(vkd, &target.textures[i].image, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    return target;
}

void vklib_render_target_recreate(vklibd* vkd, vklib_render_target* target)
{
    assume(vkd && target);

    vklib_render_target_create_info info = {};
    info.render_pass = target->render_pass;
    info.max_frame_count = target->frame_count;

    vklib_render_target_destroy(vkd, target);

    *target = vklib_render_target_create(vkd, info);
}

void vklib_render_target_destroy(vklibd* vkd, vklib_render_target* target)
{
    assume(vkd && target);

    vklib_framebuffer_destroy(vkd, target->frame_count, target->framebuffers);
    free(target->framebuffers);

    if (target->texture_count != target->frame_count)
        return;

    for (uint32_t i = 0; i < target->texture_count; i++)
    {
        vklib_image_view_destroy(vkd, target->textures[i].image_view);
        vklib_image_destroy(vkd, &target->textures[i].image);
    }
    free(target->textures);
}
