#include "vk.h"
#include "vkdev.h"
#include "vkutil.h"
#include "vkdisplay.h"

#include <stdlib.h>

vklib_display_swapchain_info vklib_display_query_swapchain_info(VkSurfaceKHR surface, VkPhysicalDevice dev, bool fill)
{
    vklib_display_swapchain_info info = {};
    assume(dev != VK_NULL_HANDLE, info);

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, surface, &info.capabilities);
    vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &info.format_count, NULL);
    if (info.format_count && fill)
    {
        info.formats = malloc(sizeof(VkSurfaceFormatKHR)*info.format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &info.format_count, info.formats);
    }
    vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &info.present_mode_count, NULL);
    if (info.present_mode_count && fill)
    {
        info.present_modes = malloc(sizeof(VkPresentModeKHR)*info.present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &info.present_mode_count, info.present_modes);
    }

    return info;
}

void vklib_display_swapchain_info_dispose(vklib_display_swapchain_info* info)
{
    assume(info);

    assume(info->formats);
    free(info->formats);

    assume(info->present_modes);
    free(info->present_modes);
}

bool vklib_display_create_swapchain(GLFWwindow* window, vk_data* vkd)
{
    assume(vkd, false);
    assume(vkd->physical_device != VK_NULL_HANDLE, false);

    vklib_display_swapchain_info info = vklib_display_query_swapchain_info(vkd->surface, vkd->physical_device, true);
    VkSurfaceFormatKHR surface_format = vklib_util_choose_swap_surface_format(info.formats, info.format_count);
    VkPresentModeKHR present_mode = vklib_util_choose_swap_present_mode(info.present_modes, info.present_mode_count);
    VkExtent2D extent = vklib_util_choose_swap_extent(window, info.capabilities);

    vkd->swapchain_image_count = info.capabilities.minImageCount + 1;
    if (info.capabilities.maxImageCount > 0 && vkd->swapchain_image_count > info.capabilities.maxImageCount)
        vkd->swapchain_image_count = info.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = vkd->surface;
    create_info.minImageCount = vkd->swapchain_image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    vklib_dev_queue_family family = vklib_dev_find_queue_families(vkd->surface, vkd->physical_device);
    uint32_t queue_family_indices[] = {
        family.graphics, family.presentation
    };

    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (family.graphics != family.presentation)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = sizeof(queue_family_indices) / sizeof(queue_family_indices[0]);
        create_info.pQueueFamilyIndices = queue_family_indices;
    }

    create_info.preTransform = info.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    vklib_display_swapchain_info_dispose(&info);

    if (vkCreateSwapchainKHR(vkd->device, &create_info, NULL, &vkd->swapchain) != VK_SUCCESS)
    {
        vkd->swapchain_image_count = 0;
        return false;
    }

    vkGetSwapchainImagesKHR(vkd->device, vkd->swapchain, &vkd->swapchain_image_count, NULL);
    vkd->swapchain_images = malloc(sizeof(VkImage) * vkd->swapchain_image_count);
    vkd->swapchain_image_views = malloc(sizeof(VkImageView) * vkd->swapchain_image_count);
    vkGetSwapchainImagesKHR(vkd->device, vkd->swapchain, &vkd->swapchain_image_count, vkd->swapchain_images);

    vkd->swapchain_format = surface_format.format;
    vkd->swapchain_extent = extent;

    for (uint32_t i = 0; i < vkd->swapchain_image_count; i++)
    {
        VkImageViewCreateInfo img_view_create_info = {};
        img_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        img_view_create_info.image = vkd->swapchain_images[i];
        img_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        img_view_create_info.format = vkd->swapchain_format;
        img_view_create_info.components.r = img_view_create_info.components.g = img_view_create_info.components.b = img_view_create_info.components.a = 
            VK_COMPONENT_SWIZZLE_IDENTITY;

        img_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        img_view_create_info.subresourceRange.baseMipLevel = 0;
        img_view_create_info.subresourceRange.levelCount = 1;
        img_view_create_info.subresourceRange.baseArrayLayer = 0;
        img_view_create_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vkd->device, &img_view_create_info, NULL, &vkd->swapchain_image_views[i]) != VK_SUCCESS)
            return false;
    }

    return true;
}

void vklib_display_dispose_swapchain(vk_data* vkd)
{
    assume(vkd);
    assume(vkd->swapchain);

    for (uint32_t i = 0; i < vkd->swapchain_image_count; i++)
        vkDestroyImageView(vkd->device, vkd->swapchain_image_views[i], NULL);

    if (vkd->swapchain_images)
        free(vkd->swapchain_images);
    if (vkd->swapchain_image_views)
        free(vkd->swapchain_image_views);

    vkDestroySwapchainKHR(vkd->device, vkd->swapchain, NULL);
}
