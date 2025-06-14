cmake_minimum_required(VERSION 3.10)
project(WaterSim)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

include_directories(include)
add_executable(water_sim
    src/main.cpp
    src/shader_utils.cpp
    src/grid.cpp
    src/camera.cpp
)

target_link_libraries(water_sim
    OpenGL::GL
    GLEW::GLEW
    glfw
    glm
)
