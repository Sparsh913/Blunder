cmake_minimum_required(VERSION 3.11)
cmake_policy(SET CMP0079 NEW)
project(laplacian)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(../../public/src ${CMAKE_BINARY_DIR}/public)
add_library(laplacian_editing laplacian_editing.cpp laplacian_editing.h)
add_library(mesh mesh.cpp mesh.h)
add_library(smoother smoother.cpp smoother.h)

add_executable(laplacian MACOSX_BUNDLE stlviewer.cpp mesh.cpp mesh.h laplacian_editing.cpp laplacian_editing.h glutil.cpp glutil.h smoother.cpp smoother.h nate.h nate.cpp)
target_link_libraries(laplacian fssimplewindow ysbitmapfont ysclass)