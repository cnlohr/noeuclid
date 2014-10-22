cmake_minimum_required(VERSION 2.8)
set(CMAKE_BUILD_TYPE Debug)
aux_source_directory(. files)
project (noeuclid)
add_executable(noeuclid ${files})
add_definitions(-std=c++11)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)


add_definitions(-DGLEW_STATIC -DGLUT_STATIC -DGL_GLEXT_PROTOTYPES -DGLUT_DISABLE_ATEXIT_HACK)
include_directories(${GLEW_INCLUDE_PATH} ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS})
target_link_libraries(noeuclid ${GLEW_LIBRARY} ${GLUT_LIBRARY} ${OPENGL_LIBRARIES} -ldl)
