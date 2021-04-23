// TODO: fix whatever the fuck is only allowing me to draw 3 shapes

#include <hvulk.hpp>

#include <iostream>
#include <stdexcept>
#include <vector>

const int WIDTH = 800;
const int HEIGHT = 800;

class TestApp : public Application
{
public:

protected:

    virtual void start(Window& primaryWindow)
    {
        primaryWindow.setSize(WIDTH, HEIGHT);
        primaryWindow.setTitle("Title");

        primaryWindow.show();

        Offset o1 = {100, 100};
        Offset o2 = {500, 500};
        Offset o3 = {100, 500};
        Offset o4 = {500, 100};

        Dimensions d = {200, 200};
        Dimensions max = {WIDTH, HEIGHT};

        Rect r(o1, d, Color::red());
        Rect r2(o2, d, Color::green());
        Rect r3(o3, d, Color::blue());
        Rect r4(o4, d, Color::white());

        primaryWindow.addShape(r);
        primaryWindow.addShape(r2);
        primaryWindow.addShape(r3);
        primaryWindow.addShape(r4);
    }

private:
};

int main()
{

    TestApp ta;
    ta.run();

    return 0;
}