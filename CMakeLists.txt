cmake_minimum_required(VERSION 3.10)
project(water_sim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Trouver OpenGL, GLEW et FreeGLUT
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)

# Inclure les headers
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${GLEW_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIR}
)

# Rassembler les sources
file(GLOB SRC src/*.cpp)

# Créer l'exécutable
add_executable(water_sim ${SRC})

# Lier les bibliothèques
target_link_libraries(water_sim PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLUT_LIBRARIES}
)
