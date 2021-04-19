#pragma once

#include <color.hpp>
#include <vertex.hpp>

struct Offset
{
    int x;
    int y;
};

struct Dimensions
{
    int w;
    int h;
};

enum Shape_Type
{
    SHAPE_TYPE_RECT
};

class Shape
{
public:

    Shape();
    Shape(Offset offset);
    Shape(Color color);
    ~Shape();

    Shape_Type getType();

protected:

    Shape_Type type;
    Offset offset;
    Color color;

private:

};

class Rect : public Shape
{
public:

    Rect();
    Rect(Offset offset, Dimensions dimensions);
    Rect(Offset offset, Dimensions dimensions, Color color);
    ~Rect();

    static std::vector<Vertex> getVertices(Rect* rect, Dimensions dim);
    static std::vector<uint16_t> getIndices();

private:
    Dimensions dimensions;
};