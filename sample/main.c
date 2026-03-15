#include "vklib/vk.h"
#include "vklib/vkpipeline.h"

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <time.h>

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

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);

    vk_data vkd = vklib_init(window, log_msg);

    size_t size = 0;
    void* data = NULL;
    VkShaderModule vertex_shader, fragment_shader;
    using(data,
        data = get_file_contents("assets/triangle.vert.spv", &size);
        vertex_shader = vklib_pipeline_shader_module_create(&vkd, data, size);
    );
    using(data,
        data = get_file_contents("assets/triangle.frag.spv", &size);
        fragment_shader = vklib_pipeline_shader_module_create(&vkd, data, size);
    );

    vklib_pipeline pipeline = vklib_pipeline_create(&vkd, vertex_shader, fragment_shader, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    vklib_pipeline_dispose(&vkd, &pipeline);
    vklib_pipeline_shader_module_dispose(&vkd, vertex_shader);
    vklib_pipeline_shader_module_dispose(&vkd, fragment_shader);

    vklib_dispose(&vkd);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
