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
    renderer.renders_finished = malloc(sizeof(VkSemaphore)*renderer.max_frames_in_flight);
    renderer.frames_in_flight = malloc(sizeof(VkFence)*renderer.max_frames_in_flight);

    for (size_t i = 0; i < renderer.max_frames_in_flight; i++)
    {
        VkSemaphoreCreateInfo semaphore_info = {};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fence_info = {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        assume(vkCreateSemaphore(vkd->device, &semaphore_info, NULL, &renderer.images_available[i]) == VK_SUCCESS, (vklib_renderer){});
        assume(vkCreateSemaphore(vkd->device, &semaphore_info, NULL, &renderer.renders_finished[i]) == VK_SUCCESS, (vklib_renderer){});
        assume(vkCreateFence(vkd->device, &fence_info, NULL, &renderer.frames_in_flight[i]) == VK_SUCCESS, (vklib_renderer){});
    }

    return renderer;
}

VkCommandBuffer vklib_renderer_get_current_cmd_buffer(vklib_renderer* renderer)
{
    return renderer->cmd.buffers[renderer->frame];
}

void vklib_renderer_begin(vklibd* vkd, vklib_renderer* renderer, VkClearValue clear_color)
{
    assume(vkd && renderer);

    VkCommandBuffer buffer = vklib_renderer_get_current_cmd_buffer(renderer);
    VkFence* frame_in_flight = &renderer->frames_in_flight[renderer->frame];
    VkSemaphore image_available = renderer->images_available[renderer->frame];
    VkSemaphore render_finished = renderer->renders_finished[renderer->frame];

    vkWaitForFences(vkd->device, 1, frame_in_flight, VK_TRUE, UINT64_MAX);
    vkAcquireNextImageKHR(vkd->device, vkd->swapchain, UINT64_MAX, image_available, VK_NULL_HANDLE, &renderer->image);

    vkResetFences(vkd->device, 1, frame_in_flight);
    vkResetCommandBuffer(buffer, 0);

    vklib_cmd_begin(&renderer->cmd, renderer->pipeline, renderer->frame);

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

        vkCmdBeginRenderPass(buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline->handle);
    }
}

void vklib_renderer_end(vklibd* vkd, vklib_renderer* renderer)
{
    assume(vkd && renderer);

    VkCommandBuffer buffer = vklib_renderer_get_current_cmd_buffer(renderer);
    VkFence* frame_in_flight = &renderer->frames_in_flight[renderer->frame];
    VkSemaphore image_available = renderer->images_available[renderer->frame];
    VkSemaphore render_finished = renderer->renders_finished[renderer->frame];

    /// DRAW CODE
    {
        vkCmdEndRenderPass(buffer);
    }

    vklib_cmd_end(&renderer->cmd, renderer->frame);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {image_available};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &buffer;

    VkSemaphore signal_semaphores[] = {render_finished};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    assume(vkQueueSubmit(vkd->graphics_queue, 1, &submit_info, renderer->frames_in_flight[renderer->frame]) == VK_SUCCESS);

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

    renderer->frame = (renderer->frame + 1) % renderer->max_frames_in_flight;
}

void vklib_renderer_destroy(vklibd* vkd, vklib_renderer* renderer)
{
    assume(vkd && renderer);

    vkDeviceWaitIdle(vkd->device);

    for (size_t i = 0; i < renderer->max_frames_in_flight; i++)
    {
        vkDestroySemaphore(vkd->device, renderer->images_available[i], NULL);
        vkDestroySemaphore(vkd->device, renderer->renders_finished[i], NULL);
        vkDestroyFence(vkd->device, renderer->frames_in_flight[i], NULL);
    }

    vklib_cmd_destroy(vkd, &renderer->cmd);
}
