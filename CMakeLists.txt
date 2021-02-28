cmake_minimum_required(VERSION 3.17)
project(VulkanCube)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(GLFW_INCLUDE_VULKAN)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE HPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*)
file(GLOB_RECURSE CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/source/*)

find_package(Vulkan)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(VulkanCube main.cpp ${HPP_FILES} ${CPP_FILES})
target_link_libraries(VulkanCube ${CONAN_LIBS} Vulkan::Vulkan)
