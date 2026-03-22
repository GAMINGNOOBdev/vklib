#include "vklib/vk.h"
#include "vklib/vkbuffer.h"
#include "vklib/vkpipeline.h"
#include "vklib/vkrenderer.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <time.h>
#include <vector>
#include <vulkan/vulkan_core.h>

#include <imgui.h>
#include "vertex.hpp"
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

void log_msg(uint8_t lvl, const char* msg, const char* file, int line)
{
    time_t local_time = time(NULL);
    struct tm* tm = localtime(&local_time);
    fprintf(stdout, "%s[%02d:%02d:%02d] %s(%s:%d): %s%s\n", LOG_COLORS[lvl+1], tm->tm_hour, tm->tm_min, tm->tm_sec, LOG_LEVEL_STRINGS[lvl], file, line, msg, LOG_COLORS[0]);
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

void render_frame(vklibd* vkd, vklib_renderer* renderer, vklib_buffer* vertexbuffer)
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

    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, &vertexbuffer->buffer, offsets);

    vkCmdDraw(cmd, vertexbuffer->size / sizeof(Vertex), 1, 0, 0);
}

void resize_callback(GLFWwindow* window, int width, int height)
{
    vklibd* vkd = reinterpret_cast<vklibd*>(glfwGetWindowUserPointer(window));
    vkd->window_resized = true;
}

ImGuiIO* mImGuiIO = NULL;
ImGuiStyle* mImGuiStyle = NULL;
ImGuiContext* mImGuiContext = NULL;

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

    vklibd vkd = vklib_init(init_data);
    glfwSetWindowUserPointer(window, &vkd);
    glfwSetFramebufferSizeCallback(window, resize_callback);

    vklib_pipeline_create_info pipeline_info = {};

    size_t size = 0;
    void* data = NULL;
    using(data,
        data = get_file_contents("assets/buffered.vert.spv", &size);
        pipeline_info.vertex = vklib_pipeline_shader_module_create(&vkd, data, size);
    );
    using(data,
        data = get_file_contents("assets/basic.frag.spv", &size);
        pipeline_info.fragment = vklib_pipeline_shader_module_create(&vkd, data, size);
    );

    pipeline_info.draw_mode = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipeline_info.wireframe = false;
    pipeline_info.vertex_size = sizeof(Vertex);
    auto vertex_attrib_info = Vertex::GetAttributeInfo();
    pipeline_info.vertex_attrib_info = vertex_attrib_info.data();
    pipeline_info.vertex_attrib_info_count = vertex_attrib_info.size();

    const std::vector<Vertex> vertices = {
        {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    vklib_buffer_create_info vbo_info = {};
    vbo_info.size = sizeof(Vertex) * vertices.size();
    vbo_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vbo_info.mode = VK_SHARING_MODE_EXCLUSIVE;
    vbo_info.memflags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vklib_buffer vbo = vklib_buffer_create(&vkd, &vbo_info);

    vklib_buffer_fill_data(&vkd, &vbo, vertices.data(), -1);

    vklib_pipeline pipeline = vklib_pipeline_create(&vkd, &pipeline_info);
    vklib_framebuffers_init(&vkd, pipeline.render_pass);

    vklib_renderer renderer = vklib_renderer_create(&vkd, &pipeline, 2);

    VkClearValue clear_color = {};
    clear_color.color.float32[0] = 0;
    clear_color.color.float32[1] = 0;
    clear_color.color.float32[2] = 0;
    clear_color.color.float32[3] = 1;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if (vklib_renderer_begin(&vkd, &renderer, clear_color))
        {
            render_frame(&vkd, &renderer, &vbo);

            vklib_renderer_end(&vkd, &renderer);
        }
    }

    vklib_buffer_destroy(&vkd, &vbo);

    vklib_renderer_destroy(&vkd, &renderer);

    vklib_pipeline_destroy(&vkd, &pipeline);
    vklib_pipeline_shader_module_destroy(&vkd, pipeline_info.vertex);
    vklib_pipeline_shader_module_destroy(&vkd, pipeline_info.fragment);

    vklib_destroy(&vkd);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
