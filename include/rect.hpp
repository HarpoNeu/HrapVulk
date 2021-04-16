#pragma once

#include <color.hpp>
#include <offset.hpp>

#include <glm/glm.hpp>

struct Rect
{
    Offset offset;
    Color color;
    int width;
    int height;
};