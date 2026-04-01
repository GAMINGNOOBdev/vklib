#include "vklib/vk.h"
#include "vklib/vktexture.h"
#include "vklib/vkuniform.h"
#include "vklib/vkpipeline.h"
#include "vklib/vkrenderer.h"
#include "vklib/vkindexbuffer.h"
#include "vklib/vkrendertarget.h"
#include "vklib/vkvertexbuffer.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <time.h>
#include <chrono>
#include <vector>
#include <vulkan/vulkan_core.h>

#include <imgui.h>
#include "camera.hpp"
#include "vertex.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define using(var, ...) \
    __VA_ARGS__; \
    free((void*)var)

#define LOG_COLOR_NONE      "\033[0m"
#define LOG_COLOR_INFO      "\033[32m"
#define LOG_COLOR_DEBUG     "\033[34m"
#define LOG_COLOR_ERROR     "\033[31m"
#define LOG_COLOR_WARNING   "\033[33m"

static const char* LOG_COLORS[] = {
    LOG_COLOR_NONE,
    LOG_COLOR_INFO,
    LOG_COLOR_DEBUG,
    LOG_COLOR_ERROR,
    LOG_COLOR_WARNING,
};

static const char* LOG_LEVEL_STRINGS[] = {
    "[ INFO ]  ",
    "[ DEBUG ] ",
    "[ ERROR ] ",
    "[WARNING] "
};

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

typedef struct
{
    vklibd* vkd;
    Camera* cam;
} resize_data_t;

FILE* LOGFILE;

void log_msg(uint8_t lvl, const char* msg, const char* file, int line)
{
    time_t local_time = time(NULL);
    struct tm* tm = localtime(&local_time);
    fprintf(stdout, "%s[%02d:%02d:%02d] %s(%s:%d): %s%s\n", LOG_COLORS[lvl+1], tm->tm_hour, tm->tm_min, tm->tm_sec, LOG_LEVEL_STRINGS[lvl], file, line, msg, LOG_COLORS[0]);
    fprintf(LOGFILE, "[%02d:%02d:%02d] %s(%s:%d): %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, LOG_LEVEL_STRINGS[lvl], file, line, msg);
}

void* get_file_contents(const char* filename, size_t* sizeptr)
{
    size_t size = 0;

    FILE* file = fopen(filename, "rb");
    if (file == NULL)
        return NULL;

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    if (sizeptr)
        *sizeptr = size;
    rewind(file);

    void* result = malloc(size+1);
    memset(result, 0, size+1);
    fread(result, 1, size, file);

    fclose(file);

    return result;
}

vklib_texture load_texture(vklibd* vkd, vklib_cmd* cmd, const char* filepath, VkSampler sampler)
{
    vklib_texture_create_info create_info = {};
    int w, h, c;
    create_info.data = stbi_load(filepath, &w, &h, &c, STBI_rgb_alpha);
    create_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    create_info.data_size = w * h * 4;
    create_info.miplevel = 1;
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.size.width = w;
    create_info.size.height = h;
    create_info.size.depth = 1;
    create_info.sampler = sampler;

    vklib_texture texture = vklib_texture_create(vkd, cmd, create_info);
    stbi_image_free(create_info.data);

    return texture;
}

void render_frame(vklibd* vkd, vklib_renderer* renderer, vklib_vertex_buffer* vbo, vklib_index_buffer* ibo)
{
    assume(vkd && renderer);

    VkCommandBuffer cmd = vklib_renderer_get_current_cmd_buffer(renderer);

    VkViewport viewport = {};
    viewport.x = viewport.y = 0;
    viewport.width = (float)vkd->swapchain_extent.width;
    viewport.height = (float)vkd->swapchain_extent.height;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0,0},
        .extent = vkd->swapchain_extent
    };
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vklib_index_buffer_render(ibo, vbo, cmd);
}

void resize_callback(GLFWwindow* window, int width, int height)
{
    resize_data_t resize_data = *reinterpret_cast<resize_data_t*>(glfwGetWindowUserPointer(window));

    resize_data.vkd->window_resized = true;
    if (resize_data.cam)
        resize_data.cam->Resize(width, height);
}

ImGuiIO* mImGuiIO = NULL;
ImGuiStyle* mImGuiStyle = NULL;
ImGuiContext* mImGuiContext = NULL;

VkSampler global_texture_sampler;

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "vulkan sample app", NULL, NULL);

    vklibd_init_data init_data = {
        .window = window,
        .app_name = "vulkan sample app",
        .engine_name = "None",
        .logfunc = log_msg,
    };

    LOGFILE = fopen("vklog.log", "a+");

    vklibd vkd = vklib_init(init_data);
    if (vkd.instance == VK_NULL_HANDLE)
    {
        log_msg(LOGLEVEL_ERROR, "unable to initialize vulkan", __FILE_NAME__, __LINE__);
        fclose(LOGFILE);
        return EXIT_FAILURE;
    }

    resize_data_t resize_data = {
        &vkd,NULL
    };
    glfwSetWindowUserPointer(window, &resize_data);
    glfwSetFramebufferSizeCallback(window, resize_callback);

    VkRenderPass render_pass = vklib_render_pass_create(&vkd);

    vklib_pipeline_create_info pipeline_info = {};
    pipeline_info.render_pass = render_pass;

    size_t size = 0;
    void* data = NULL;
    using(data,
        data = get_file_contents("assets/textured.vert.spv", &size);
        pipeline_info.vertex = vklib_pipeline_shader_module_create(&vkd, data, size);
    );
    using(data,
        data = get_file_contents("assets/textured.frag.spv", &size);
        pipeline_info.fragment = vklib_pipeline_shader_module_create(&vkd, data, size);
    );

    pipeline_info.draw_mode = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipeline_info.wireframe = false;
    pipeline_info.vertex_size = sizeof(Vertex);
    auto vertex_attrib_info = Vertex::GetAttributeInfo();
    pipeline_info.vertex_attrib_info = vertex_attrib_info.data();
    pipeline_info.vertex_attrib_info_count = vertex_attrib_info.size();

    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    };
    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    Camera camera = Camera(WIDTH, HEIGHT);
    resize_data.cam = &camera;

    std::vector<VkDescriptorSetLayoutBinding> uniform_bindings = {
        camera.data.GetBindingInfo(),
        {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = NULL,
        }
    };

    pipeline_info.uniform_binding_count = uniform_bindings.size();
    pipeline_info.uniform_bindings = uniform_bindings.data();

    vklib_pipeline pipeline = vklib_pipeline_create(&vkd, pipeline_info);

    vklib_render_target_create_info render_target_info = {};
    render_target_info.render_pass = render_pass;
    render_target_info.max_frame_count = vkd.swapchain_image_count;
    vklib_render_target render_target = vklib_render_target_create(&vkd, render_target_info);

    vklib_renderer renderer = vklib_renderer_create(&vkd, &pipeline, MAX_FRAMES_IN_FLIGHT);

    vklib_vertex_buffer vbo = vklib_vertex_buffer_create(&vkd, &renderer.cmd, vertices.data(), sizeof(Vertex), vertices.size());
    vklib_index_buffer ibo = vklib_index_buffer_create(&vkd, &renderer.cmd, indices.data(), VK_INDEX_TYPE_UINT16, sizeof(uint16_t), indices.size());

    VkSamplerCreateInfo sampler_create_info = {};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.anisotropyEnable = VK_TRUE;
    sampler_create_info.maxAnisotropy = FLT_MAX;
    sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    sampler_create_info.compareEnable = VK_FALSE;
    sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.mipLodBias = 0.0f;
    sampler_create_info.minLod = 0.0f;
    sampler_create_info.maxLod = 0.0f;

    global_texture_sampler = vklib_texture_sampler_create(&vkd, sampler_create_info);

    vklib_texture texture = load_texture(&vkd, &renderer.cmd, "assets/test1.png", global_texture_sampler);

    std::array<vklib_uniform_descriptor_create_info, 2> uniform_buffer_descriptors = {};
    uniform_buffer_descriptors[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_buffer_descriptors[0].size = sizeof(camera.data);

    uniform_buffer_descriptors[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    uniform_buffer_descriptors[1].image = &texture;

    vklib_uniform_create_info uniform_buffer_create_info = {};
    uniform_buffer_create_info.descriptor_count = uniform_buffer_descriptors.size();
    uniform_buffer_create_info.descriptors = uniform_buffer_descriptors.data();
    uniform_buffer_create_info.pipeline = &pipeline;
    uniform_buffer_create_info.renderer = &renderer;
    vklib_uniform ubo = vklib_uniform_create(&vkd, uniform_buffer_create_info);

    VkClearValue clear_color = {};
    clear_color.color.float32[0] = 0;
    clear_color.color.float32[1] = 0;
    clear_color.color.float32[2] = 0;
    clear_color.color.float32[3] = 1;
    static auto startTime = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(window))
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        camera.Update(time);

        glfwPollEvents();

        vklib_uniform_update_descriptor(&ubo, renderer.frame, 0, &camera.data, sizeof(camera.data));

        if (vklib_renderer_begin(&vkd, &renderer, &render_target, clear_color))
        {
            vklib_uniform_bind(&ubo, &renderer);
            render_frame(&vkd, &renderer, &vbo, &ibo);

            vklib_renderer_end(&vkd, &renderer, &render_target);
        }
    }

    vklib_renderer_destroy(&vkd, &renderer);

    vklib_vertex_buffer_destroy(&vkd, &vbo);
    vklib_index_buffer_destroy(&vkd, &ibo);

    vklib_texture_destroy(&vkd, &texture);

    vklib_uniform_destroy(&vkd, &ubo);

    vklib_texture_sampler_destroy(&vkd, global_texture_sampler);

    vklib_pipeline_destroy(&vkd, &pipeline);
    vklib_pipeline_shader_module_destroy(&vkd, pipeline_info.vertex);
    vklib_pipeline_shader_module_destroy(&vkd, pipeline_info.fragment);
    vklib_render_target_destroy(&vkd, &render_target);
    vklib_render_pass_destroy(&vkd, render_pass);

    vklib_destroy(&vkd);

    fclose(LOGFILE);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
