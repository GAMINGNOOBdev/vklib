#ifndef __VERTEX_H_
#define __VERTEX_H_

#include <array>
#include <glm/glm.hpp>
#include <vklib/vkpipeline.h>

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;

    static inline std::array<vklib_pipeline_vertex_attribute_info, 2> GetAttributeInfo()
    {
        std::array<vklib_pipeline_vertex_attribute_info, 2> attribute_info;

        attribute_info[0].binding = 0;
        attribute_info[0].location = 0;
        attribute_info[0].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_info[0].offset = offsetof(struct Vertex, pos);

        attribute_info[1].binding = 0;
        attribute_info[1].location = 1;
        attribute_info[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_info[1].offset = offsetof(struct Vertex, color);
        return attribute_info;
    }
};

#endif
