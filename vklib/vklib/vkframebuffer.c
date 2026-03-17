#include "vklib/vk.h"
#include "vklib/vkframebuffer.h"

#include <vulkan/vulkan_core.h>

void vklib_framebuffer_create(vklibd* vkd, VkRenderPass render_pass, int n, VkFramebuffer* buffers, VkImageView* image_views)
{
    assume(vkd && n && buffers && render_pass && image_views);

    for (int i = 0; i < n; i++)
    {
        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = &image_views[i];
        framebuffer_info.width = vkd->swapchain_extent.width;
        framebuffer_info.height = vkd->swapchain_extent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(vkd->device, &framebuffer_info, NULL, &buffers[i]) != VK_SUCCESS)
        {
            LOGERROR("unable to create framebuffer no. %d", i);
            return;
        }
    }
}

void vklib_framebuffer_destroy(vklibd* vkd, int n, VkFramebuffer* buffers)
{
    assume(vkd && n && buffers);

    for (int i = 0; i < n; i++)
        vkDestroyFramebuffer(vkd->device, buffers[i], NULL);
}
