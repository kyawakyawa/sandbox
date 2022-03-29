cmake_minimum_required(VERSION 3.8)
project(clspv_test)

find_package(Vulkan)

add_library(deps STATIC ${PROJECT_SOURCE_DIR}/deps/load_png/lodepng.cpp)
target_compile_features(deps PRIVATE cxx_std_11)
target_include_directories(deps PUBLIC ${PROJECT_SOURCE_DIR}/deps/load_png)
