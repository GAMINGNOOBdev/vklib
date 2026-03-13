#include "assert.h"
#include "vk.h"

vk_data vklib_init(GLFWwindow* window)
{
    vk_data vkd = {};
    if (volkInitialize() != VK_SUCCESS)
        return vkd;

    return vkd;
}

void vklib_dispose(vk_data* vkd)
{
    assert(vkd);
}
