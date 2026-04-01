#include "vklib/vk.h"
#include "vklib/vkcmd.h"
#include "vklib/vkrenderer.h"

#include <stdlib.h>
#include <memory.h>
#include <vulkan/vulkan_core.h>

vklib_renderer vklib_renderer_create(vklibd* vkd, vklib_pipeline* pipeline, uint32_t max_frames_in_flight)
{
    vklib_renderer renderer = {};
    assume(vkd, renderer);

    renderer.max_frames_in_flight = max_frames_in_flight;
    if (renderer.max_frames_in_flight > vkd->swapchain_image_count)
        renderer.max_frames_in_flight = vkd->swapchain_image_count;

    renderer.cmd = vklib_cmd_create(vkd, renderer.max_frames_in_flight);

    renderer.pipeline = pipeline;

    renderer.images_available = malloc(sizeof(VkSemaphore)*renderer.max_frames_in_flight);
    renderer.frames_in_flight = malloc(sizeof(VkFence)*renderer.max_frames_in_flight);
    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < renderer.max_frames_in_flight; i++)
    {
        assume(vkCreateSemaphore(vkd->device, &semaphore_info, NULL, &renderer.images_available[i]) == VK_SUCCESS, (vklib_renderer){});
        assume(vkCreateFence(vkd->device, &fence_info, NULL, &renderer.frames_in_flight[i]) == VK_SUCCESS, (vklib_renderer){});
    }
    renderer.renders_finished = malloc(sizeof(VkSemaphore)*vkd->swapchain_image_count);
    for (size_t i = 0; i < vkd->swapchain_image_count; i++)
        assume(vkCreateSemaphore(vkd->device, &semaphore_info, NULL, &renderer.renders_finished[i]) == VK_SUCCESS, (vklib_renderer){});

    return renderer;
}

VkCommandBuffer vklib_renderer_get_current_cmd_buffer(vklib_renderer* renderer)
{
    return renderer->cmd.buffers[renderer->frame];
}

#define image_available renderer->images_available[renderer->frame]
#define frame_in_flight renderer->frames_in_flight[renderer->frame]

bool vklib_renderer_begin(vklibd* vkd, vklib_renderer* renderer, vklib_render_target* render_target, VkClearValue clear_color)
{
    assume(vkd && renderer && render_target, false);

    VkCommandBuffer buffer = vklib_renderer_get_current_cmd_buffer(renderer);

    vkWaitForFences(vkd->device, 1, &frame_in_flight, VK_TRUE, UINT64_MAX);
    VkResult result = vkAcquireNextImageKHR(vkd->device, vkd->swapchain, UINT64_MAX, image_available, VK_NULL_HANDLE, &renderer->image);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        vklib_handle_view_changes(vkd);
        vklib_render_target_recreate(vkd, render_target);
        return false;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        LOGERROR("unable to acquire swapchain image");
        return false;
    }

    vkResetFences(vkd->device, 1, &frame_in_flight);
    vkResetCommandBuffer(buffer, 0);

    vklib_cmd_begin(&renderer->cmd, renderer->frame);

    // DRAW CODE
    {
        VkRenderPassBeginInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = render_target->render_pass;
        render_pass_info.framebuffer = render_target->framebuffers[renderer->image];
        render_pass_info.renderArea.offset = (VkOffset2D){0,0};
        render_pass_info.renderArea.extent = render_target->extent;

        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clear_color;

        vkCmdBeginRenderPass(buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline->handle);
    }

    return true;
}

#define render_finished renderer->renders_finished[renderer->image]

bool vklib_renderer_end(vklibd* vkd, vklib_renderer* renderer, vklib_render_target* render_target)
{
    assume(vkd && renderer, false);

    VkCommandBuffer buffer = vklib_renderer_get_current_cmd_buffer(renderer);

    /// DRAW CODE
    {
        vkCmdEndRenderPass(buffer);
    }

    vklib_cmd_end(&renderer->cmd, renderer->frame);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &image_available;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &buffer;

    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &render_finished;

    assume(vkQueueSubmit(vkd->graphics_queue, 1, &submit_info, frame_in_flight) == VK_SUCCESS, false);

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_finished;

    present_info.swapchainCount = 1;
    present_info.pSwapchains = &vkd->swapchain;
    present_info.pImageIndices = &renderer->image;
    present_info.pResults = NULL;

    VkResult result = vkQueuePresentKHR(vkd->presentation_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vkd->window_resized)
    {
        vkd->window_resized = false;
        vklib_handle_view_changes(vkd);
        vklib_render_target_recreate(vkd, render_target);
    }
    else if (result != VK_SUCCESS)
    {
        LOGERROR("unable to present swapchain image");
        return false;
    }

    renderer->frame = (renderer->frame + 1) % renderer->max_frames_in_flight;
    return true;
}

#undef render_finished
#undef image_available
#undef frame_in_flight

void vklib_renderer_destroy(vklibd* vkd, vklib_renderer* renderer)
{
    assume(vkd && renderer);

    vkDeviceWaitIdle(vkd->device);

    for (size_t i = 0; i < renderer->max_frames_in_flight; i++)
    {
        vkDestroySemaphore(vkd->device, renderer->images_available[i], NULL);
        vkDestroyFence(vkd->device, renderer->frames_in_flight[i], NULL);
    }
    for (size_t i = 0; i < vkd->swapchain_image_count; i++)
        vkDestroySemaphore(vkd->device, renderer->renders_finished[i], NULL);

    vklib_cmd_destroy(vkd, &renderer->cmd);
}
