#include "vklib/vk.h"
#include "vklib/vkrenderer.h"

#include <vulkan/vulkan_core.h>

vklib_renderer vklib_renderer_create(vklibd* vkd, vklib_pipeline* pipeline, vklib_cmd* cmd)
{
    vklib_renderer renderer = {};
    assume(vkd, renderer);

    renderer.cmd = cmd;
    renderer.pipeline = pipeline;

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    assume(vkCreateSemaphore(vkd->device, &semaphore_info, NULL, &renderer.image_available) == VK_SUCCESS, (vklib_renderer){});
    assume(vkCreateSemaphore(vkd->device, &semaphore_info, NULL, &renderer.render_finished) == VK_SUCCESS, (vklib_renderer){});
    assume(vkCreateFence(vkd->device, &fence_info, NULL, &renderer.frame_in_flight) == VK_SUCCESS, (vklib_renderer){});

    return renderer;
}

void vklib_renderer_begin(vklibd* vkd, vklib_renderer* renderer, VkClearValue clear_color)
{
    assume(vkd && renderer);

    vkWaitForFences(vkd->device, 1, &renderer->frame_in_flight, VK_TRUE, UINT64_MAX);
    vkResetFences(vkd->device, 1, &renderer->frame_in_flight);

    vkAcquireNextImageKHR(vkd->device, vkd->swapchain, UINT64_MAX, renderer->image_available, VK_NULL_HANDLE, &renderer->image);

    vkResetCommandBuffer(renderer->cmd->buffer, 0);

    vklib_cmd_begin(renderer->cmd, renderer->pipeline);

    // DRAW CODE
    {
        VkRenderPassBeginInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = renderer->pipeline->render_pass;
        render_pass_info.framebuffer = vkd->framebuffers[renderer->image];
        render_pass_info.renderArea.offset = (VkOffset2D){0,0};
        render_pass_info.renderArea.extent = vkd->swapchain_extent;

        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clear_color;

        vkCmdBeginRenderPass(renderer->cmd->buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(renderer->cmd->buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline->handle);
    }
}

void vklib_renderer_end(vklibd* vkd, vklib_renderer* renderer)
{
    assume(vkd && renderer);

    {
        vkCmdEndRenderPass(renderer->cmd->buffer);
    }

    vklib_cmd_end(renderer->cmd);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {renderer->image_available};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &renderer->cmd->buffer;

    VkSemaphore signal_semaphores[] = {renderer->render_finished};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    assume(vkQueueSubmit(vkd->graphics_queue, 1, &submit_info, renderer->frame_in_flight) == VK_SUCCESS);

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapchains[] = {vkd->swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &renderer->image;
    present_info.pResults = NULL;

    vkQueuePresentKHR(vkd->presentation_queue, &present_info);
}

void vklib_renderer_destroy(vklibd* vkd, vklib_renderer* renderer)
{
    assume(vkd && renderer);

    vkDeviceWaitIdle(vkd->device);

    vkDestroySemaphore(vkd->device, renderer->image_available, NULL);
    vkDestroySemaphore(vkd->device, renderer->render_finished, NULL);
    vkDestroyFence(vkd->device, renderer->frame_in_flight, NULL);
}
