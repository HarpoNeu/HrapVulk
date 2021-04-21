#include <color.hpp>

basic_color Color::red()
{
    return {255, 0, 0, 255};
}

basic_color Color::green()
{
    return {0, 255, 0, 255};
}

basic_color Color::blue()
{
    return {0, 0, 255, 255};
}

basic_color Color::white()
{
    return {255, 255, 255, 255};
}