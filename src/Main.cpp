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
    }

private:
};

int main()
{

    TestApp ta;
    ta.run();

    return 0;
}