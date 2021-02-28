#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <iostream>
#include "primitives.h"
//#include <fmt/format.h>
#include "VulkanCube.h"

struct MyStruct
{
    glm::vec4 a;
    float b;
    glm::vec3 c;
};

int main() {
    try {
        VulkanCube vulkanCube{};
        vulkanCube.run();
    }catch(const std::runtime_error& error){
        spdlog::error(error.what());
    }
    return 0;
}
