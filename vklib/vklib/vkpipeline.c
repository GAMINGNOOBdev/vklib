#include "vklib/vk.h"
#include "vklib/vkpipeline.h"
#include <vulkan/vulkan_core.h>

#include <memory.h>

VkShaderModule vklib_pipeline_shader_module_create(vklibd* vkd, void* data, size_t size)
{
    assume(vkd, VK_NULL_HANDLE);
    assume(vkd->instance, VK_NULL_HANDLE);
    assume(data, VK_NULL_HANDLE);
    assume(size, VK_NULL_HANDLE);
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = size;
    create_info.pCode = (const uint32_t*)data;

    VkShaderModule module;
    if (vkCreateShaderModule(vkd->device, &create_info, NULL, &module) != VK_SUCCESS)
    {
        LOGERROR("Failed to create shader module");
        return VK_NULL_HANDLE;
    }
    return module;
}

void vklib_pipeline_shader_module_destroy(vklibd* vkd, VkShaderModule module)
{
    assume(vkd);
    assume(vkd->instance);
    assume(module);

    vkDestroyShaderModule(vkd->device, module, NULL);
}

VkRenderPass vklib_pipeline_render_pass_create(vklibd* vkd)
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

    VkRenderPass render_pass = VK_NULL_HANDLE;
    if (vkCreateRenderPass(vkd->device, &render_pass_info, NULL, &render_pass) != VK_SUCCESS)
    {
        LOGERROR("failed to create render pass");
        return VK_NULL_HANDLE;
    }
    return render_pass;
}

void vklib_pipeline_render_pass_destroy(vklibd* vkd, VkRenderPass render_pass)
{
    assume(vkd && vkd->instance && render_pass);

    vkDestroyRenderPass(vkd->device, render_pass, NULL);
}

vklib_pipeline vklib_pipeline_create(vklibd* vkd, VkShaderModule vertex, VkShaderModule fragment, VkPrimitiveTopology draw_mode, bool wireframe)
{
    vklib_pipeline pipeline = {};
    assume(vkd, pipeline);
    assume(vertex && fragment, pipeline);

    pipeline.render_pass = vklib_pipeline_render_pass_create(vkd);

    VkPipelineShaderStageCreateInfo shader_stage_info[] = {
        (VkPipelineShaderStageCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertex,
            .pName = "main"
        },
        (VkPipelineShaderStageCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragment,
            .pName = "main"
        },
    };

    pipeline.vertex = vertex;
    pipeline.fragment = fragment;

    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
    dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_info.dynamicStateCount = sizeof(dynamic_states) / sizeof(dynamic_states[0]);
    dynamic_state_info.pDynamicStates = dynamic_states;

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.pVertexBindingDescriptions = NULL;
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.pVertexAttributeDescriptions = NULL;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
    input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.primitiveRestartEnable = VK_FALSE;
    input_assembly_info.topology = pipeline.draw_mode = draw_mode;

    VkPipelineViewportStateCreateInfo viewport_info = {};
    viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_info.viewportCount = 1;
    viewport_info.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer_info = {};
    rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_info.depthClampEnable = VK_FALSE;
    rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
    rasterizer_info.polygonMode = wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizer_info.lineWidth = 1.0f;
    rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer_info.depthBiasEnable = VK_FALSE;
    rasterizer_info.depthBiasConstantFactor = 0;
    rasterizer_info.depthBiasClamp = 0;
    rasterizer_info.depthBiasSlopeFactor = 0;

    VkPipelineMultisampleStateCreateInfo multisampling_info = {};
    multisampling_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling_info.sampleShadingEnable = VK_FALSE;
    multisampling_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling_info.minSampleShading = 1.0f;
    multisampling_info.pSampleMask = NULL;
    multisampling_info.alphaToCoverageEnable = VK_FALSE;
    multisampling_info.alphaToOneEnable = VK_FALSE;

    /// TODO: Add depth buffer
    /// VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {};

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blend_info = {};
    color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_info.logicOpEnable = VK_FALSE;
    color_blend_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_info.attachmentCount = 1;
    color_blend_info.pAttachments = &color_blend_attachment;
    memset(color_blend_info.blendConstants, 0, sizeof(float)*4);

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0;

    if (vkCreatePipelineLayout(vkd->device, &pipeline_layout_info, NULL, &pipeline.layout) != VK_SUCCESS)
    {
        LOGERROR("failed to create pipeline layout");
        return (vklib_pipeline){};
    }

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = sizeof(shader_stage_info) / sizeof(shader_stage_info[0]);
    pipeline_info.pStages = shader_stage_info;

    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly_info;
    pipeline_info.pViewportState = &viewport_info;
    pipeline_info.pRasterizationState = &rasterizer_info;
    pipeline_info.pMultisampleState = &multisampling_info;
    pipeline_info.pDepthStencilState = NULL; /// TODO: DEPTH BUFFER
    pipeline_info.pColorBlendState = &color_blend_info;
    pipeline_info.pDynamicState = &dynamic_state_info;

    pipeline_info.layout = pipeline.layout;

    pipeline_info.renderPass = pipeline.render_pass;
    pipeline_info.subpass = 0;

    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(vkd->device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline.handle) != VK_SUCCESS)
    {
        LOGERROR("unable to create graphics pipeline");
        return (vklib_pipeline){};
    }

    return pipeline;
}

void vklib_pipeline_destroy(vklibd* vkd, vklib_pipeline* pipeline)
{
    assume(vkd);
    assume(pipeline);

    vkDestroyPipeline(vkd->device, pipeline->handle, NULL);
    vkDestroyPipelineLayout(vkd->device, pipeline->layout, NULL);
    vklib_pipeline_render_pass_destroy(vkd, pipeline->render_pass);
}
