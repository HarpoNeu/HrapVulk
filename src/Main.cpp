#include <hvulk.hpp>

#include <iostream>
#include <stdexcept>
#include <vector>

class TestApp : public Application
{
public:

protected:

    virtual void start(Window& primaryWindow)
    {
        primaryWindow.setSize(800, 800);
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