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
        std::array<vklib_pipeline_vertex_attribute_info, 2> attributeInfo;

        attributeInfo[0].binding = 0;
        attributeInfo[0].location = 0;
        attributeInfo[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeInfo[0].offset = offsetof(struct Vertex, pos);

        attributeInfo[1].binding = 0;
        attributeInfo[1].location = 1;
        attributeInfo[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeInfo[1].offset = offsetof(struct Vertex, color);
        return attributeInfo;
    }
};

#endif
