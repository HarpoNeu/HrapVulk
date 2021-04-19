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

        Color c = {255, 0, 0, 255};

        Dimensions d = {200, 200};

        Rect r(o1, d, c);
        Rect r2(o2, d, c);

        primaryWindow.addShape(&r);
        primaryWindow.addShape(&r2);
    }

private:
};

int main()
{

    TestApp ta;
    ta.run();

    return 0;
}