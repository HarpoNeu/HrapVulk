#pragma once

struct basic_color
{
    int r;
    int g;
    int b;
    int a;
};

struct Color : public basic_color
{
    static basic_color red();
    static basic_color green();
    static basic_color blue();
    static basic_color white();
};