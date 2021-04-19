#pragma once

#include <color.hpp>
#include <dimensions.hpp>
#include <offset.hpp>
#include <vertex.hpp>

#include <vector>

struct Rect
{
    Offset offset;
    Color color;
    Dimensions dimensions;

    std::vector<Vertex> getVertices(Dimensions dim);

    static std::vector<uint16_t> getIndices();
};