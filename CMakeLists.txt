cmake_minimum_required(VERSION 2.8)

PROJECT(BouncingBall)

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
add_definitions(${GLUT_DEFINITIONS})

find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})

add_executable(BouncingBall main.cpp)


target_link_libraries(BouncingBall ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )