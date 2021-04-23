#include <shape.hpp>
#include <utils.hpp>

#include <iostream>
#include <vector>

Shape::Shape()
{
    offset = {0, 0};
    color = {0, 0, 0, 0};
}

Shape::Shape(Offset offset)
{
    this->offset = offset;
    color = {0, 0, 0, 0};
}

Shape::Shape(basic_color color)
{
    offset = {0, 0};
    this->color = color;
}

Shape::~Shape()
{

}

Shape_Type Shape::getType()
{
    return type;
}

Rect::Rect()
{
    type = SHAPE_TYPE_RECT;
    offset = {0, 0};
    dimensions = {0, 0};
    color = {0, 0, 0, 0};
}

Rect::Rect(Offset offset, Dimensions dimensions)
{
    type = SHAPE_TYPE_RECT;
    this->offset = offset;
    this->dimensions = dimensions;
    color = {0, 0, 0, 0};
}

Rect::Rect(Offset offset, Dimensions dimensions, basic_color color)
{
    type = SHAPE_TYPE_RECT;
    this->offset = offset;
    this->dimensions = dimensions;
    this->color = color;
}

Rect::~Rect()
{

}

std::vector<Vertex> Rect::getVertices(Rect rect, Dimensions dim)
{
    float w = convertPxToFl(rect.dimensions.w, dim.w) + 1;
    float h = convertPxToFl(rect.dimensions.h, dim.h) + 1;
    float x = convertPxToFl(rect.offset.x, dim.w);
    float y = convertPxToFl(rect.offset.y, dim.h);

    float r = convertColor(rect.color.r);
    float g = convertColor(rect.color.g);
    float b = convertColor(rect.color.b);
    float a = convertColor(rect.color.a);

    std::cout << rect.color.r << ", " << r << " : " << rect.color.g << ", " << g << " : " << rect.color.b << ", " << b << std::endl;;

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
