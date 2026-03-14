cmake_minimum_required(VERSION 3.10)

project(RayForge3D)

file(GLOB SRC_FILES src/*.cpp)

include_directories(src/include libraries/glfw-3.4/include libraries/glad/include)
add_subdirectory(libraries/glfw-3.4)
find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME} libraries/glad/src/gl.c ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} glfw OpenGL::GL)