#pragma once

#include <device.hpp>

void here();
VkInstance getInstance();
Device& getDevice();
std::vector<char> readFile(const std::string& fileName);
void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool pool, VkQueue queue);
VkCommandBuffer beginSingleTimeCommands(VkCommandPool pool);
void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool pool, VkQueue queue);
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);