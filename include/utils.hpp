#pragma once

#include <device.hpp>

void here();
VkInstance getInstance();

std::vector<char> readFile(const std::string& fileName);

float convertPxToFl(int px, int d);
float convertColor(int c);