#include <utils.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

#include <unistd.h>

void here()
{
    std::cout << "here" << std::endl;
}

std::vector<char> readFile(const std::string& fileName)
{
    char dir[256];
    getcwd(dir, sizeof(dir));

    std::stringstream path;
    path << dir << fileName;

    std::ifstream file(path.str(), std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        char err[255];
        sprintf(err, "Error! Failed to open file: %s", fileName.c_str());
        throw std::runtime_error(err);
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}