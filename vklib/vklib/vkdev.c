#include "vk.h"
#include "set.h"
#include "vkdev.h"
#include "vkutil.h"
#include "vkdisplay.h"

#include <stdlib.h>
#include <string.h>

const char* ENGINE_VK_DEVICE_EXTENSIONS[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static uint64_t strhash(const char* str)
{
    uint64_t identifier = 5381;
    size_t length = strlen(str);
    for (size_t i = 0; i < length; i++)
    {
        char c = str[i];
        identifier = ((identifier << 5) + identifier) + c;
    }
    return identifier;
}

bool vklib_dev_check_extensions(VkPhysicalDevice dev)
{
    uint32_t engine_extension_count = sizeof(ENGINE_VK_DEVICE_EXTENSIONS) / sizeof(ENGINE_VK_DEVICE_EXTENSIONS[0]);

    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(dev, NULL, &extension_count, NULL);
    VkExtensionProperties* extensions = malloc(sizeof(VkExtensionProperties)*extension_count);
    vkEnumerateDeviceExtensionProperties(dev, NULL, &extension_count, extensions);

    u64_set set = set_init(u64);
    for (uint32_t i = 0; i < engine_extension_count; i++)
        u64_set_append(&set, strhash(ENGINE_VK_DEVICE_EXTENSIONS[i]));

    for (uint32_t i = 0; i < extension_count; i++)
    {
        u64 hash = strhash(extensions[i].extensionName);
        u64_set_remove(&set, hash);
    }
    bool result = set.count == 0;
    u64_set_dispose(&set);
    free(extensions);
    return result;
}

bool vklib_dev_is_suitable(VkSurfaceKHR surface, VkPhysicalDevice dev)
{
    vklib_dev_queue_family family = vklib_dev_find_queue_families(surface, dev);
    bool extensions_supported = vklib_dev_check_extensions(dev);
    bool swapchain_compatible = false;
    if (extensions_supported)
    {
        vklib_display_swapchain_info info = vklib_display_query_swapchain_info(surface, dev, false);
        swapchain_compatible = info.format_count != 0 && info.present_mode_count != 0;
    }
    return family.graphics_present && family.presentation_present && extensions_supported && swapchain_compatible;
}

bool vklib_dev_pick(vk_data* vkd)
{
    assume(vkd, false);
    assume(vkd->instance, false);

    vkd->physical_device = VK_NULL_HANDLE;

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(vkd->instance, &device_count, NULL);

    assume(device_count != 0, false);

    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice)*device_count);
    vkEnumeratePhysicalDevices(vkd->instance, &device_count, devices);
    for (uint32_t i = 0; i < device_count; i++)
    {
        if (!vklib_dev_is_suitable(vkd->surface, devices[i]))
            continue;

        vkd->physical_device = devices[i];
        break;
    }
    free(devices);

    assume(vkd->physical_device != VK_NULL_HANDLE, false);

    // create logical device

    vklib_dev_queue_family indices = vklib_dev_find_queue_families(vkd->surface, vkd->physical_device);
    float priority = 1.0f;

    u32_set set = set_init(u32);
    u32_set_append(&set, indices.graphics);
    u32_set_append(&set, indices.presentation);

    VkDeviceQueueCreateInfo* queue_create_info = malloc(sizeof(VkDeviceQueueCreateInfo) * set.count);
    for (size_t i = 0; i < set.count; i++)
    {
        queue_create_info[i] = (VkDeviceQueueCreateInfo){};
        queue_create_info[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info[i].queueFamilyIndex = set.data[i];
        queue_create_info[i].queueCount = 1;
        queue_create_info[i].pQueuePriorities = &priority;
    }

    VkPhysicalDeviceFeatures device_features = {};

    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = queue_create_info;
    create_info.queueCreateInfoCount = set.count;
    create_info.ppEnabledExtensionNames = ENGINE_VK_DEVICE_EXTENSIONS;
    create_info.enabledExtensionCount = sizeof(ENGINE_VK_DEVICE_EXTENSIONS) / sizeof(ENGINE_VK_DEVICE_EXTENSIONS[0]);
    create_info.ppEnabledLayerNames = vklib_util_get_enabled_layers(&create_info.enabledLayerCount);
    create_info.pEnabledFeatures = &device_features;

    if (vkCreateDevice(vkd->physical_device, &create_info, NULL, &vkd->device) != VK_SUCCESS)
        return false;

    free(queue_create_info);
    u32_set_dispose(&set);

    vkGetDeviceQueue(vkd->device, indices.graphics, 0, &vkd->graphics_queue);
    vkGetDeviceQueue(vkd->device, indices.presentation, 0, &vkd->presentation_queue);

    return true;
}

bool vklib_dev_dispose(vk_data* vkd)
{
    assume(vkd, false);
    assume(vkd->instance, false);
    assume(vkd->physical_device != VK_NULL_HANDLE, false);

    vkDestroyDevice(vkd->device, NULL);

    return true;
}

vklib_dev_queue_family vklib_dev_find_queue_families(VkSurfaceKHR surface, VkPhysicalDevice dev)
{
    vklib_dev_queue_family indices = {};

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_count, NULL);
    VkQueueFamilyProperties* queue_families = malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_count, queue_families);

    for (uint32_t i = 0; i < queue_family_count; i++)
    {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && !indices.graphics_present)
        {
            indices.graphics = i;
            indices.graphics_present = true;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surface, &present_support);
        if (present_support)
        {
            indices.presentation = i;
            indices.presentation_present = true;
        }
    }

    free(queue_families);

    return indices;
}
