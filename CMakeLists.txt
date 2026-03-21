cmake_minimum_required(VERSION 3.10)

project(RayForge3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SRC_FILES src/*.cpp)
file(GLOB imGui libraries/imgui/*.cpp)

include_directories(
    src/include
    libraries/glfw-3.4/include
    libraries/glad/include
    libraries/
    libraries/imgui
)
add_subdirectory(libraries/glfw-3.4)
find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME} libraries/glad/src/glad.c ${SRC_FILES} ${imGui})

if(UNIX)
    set(PLATFORM_LIBS pthread dl)
endif()

target_link_libraries(${PROJECT_NAME} glfw OpenGL::GL ${PLATFORM_LIBS})
