#include <rect.hpp>

#include <utils.hpp>

std::vector<Vertex> Rect::getVertices(Dimensions dim)
{
    float w = convertPxToFl(dimensions.w, dim.w) + 1;
    float h = convertPxToFl(dimensions.h, dim.h) + 1;
    float x = convertPxToFl(offset.x, dim.w);
    float y = convertPxToFl(offset.y, dim.h);

    float r = convertColor(color.r);
    float g = convertColor(color.b);
    float b = convertColor(color.b);
    float a = convertColor(color.a);

    std::vector<Vertex> vertices;
    vertices.push_back({{x, y}, {r, g, b}});
    vertices.push_back({{x + w, y}, {r, g, b}});
    vertices.push_back({{x + w, y + h}, {r, g, b}});
    vertices.push_back({{x, y + h}, {r, g, b}});

    return vertices;
}

std::vector<uint16_t> Rect::getIndices()
{
    std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    return indices;
}
