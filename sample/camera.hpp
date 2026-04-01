#ifndef __CAMERA_H_
#define __CAMERA_H_

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>
#include <glm/gtc/matrix_transform.hpp>

struct CameraUniform
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;

    static inline VkDescriptorSetLayoutBinding GetBindingInfo()
    {
        VkDescriptorSetLayoutBinding bindingInfo = {};
        bindingInfo.binding = 0;
        bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindingInfo.descriptorCount = 1;
        bindingInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        return bindingInfo;
    }
};

struct Camera
{
    CameraUniform data;

    inline Camera(size_t width, size_t height)
    {
        data = {
            .model = glm::mat4(1.0f),
            .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        };
        Resize(width, height);
    }

    inline void Update(float time)
    {
        data.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    }

    inline void Resize(size_t width, size_t height)
    {
        data.proj = glm::perspective(glm::radians(45.0f), width / (float) height, 0.1f, 10.0f);
        data.proj[1][1] *= -1;
    }
};

#endif
