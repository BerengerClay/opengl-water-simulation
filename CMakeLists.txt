cmake_minimum_required(VERSION 3.10)
project(water_sim)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SRC src/*.cpp)

add_executable(water_sim ${SRC})
target_link_libraries(water_sim ${OPENGL_LIBRARIES} GLEW::GLEW glfw)
