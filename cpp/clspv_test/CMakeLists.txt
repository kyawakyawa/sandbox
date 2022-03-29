cmake_minimum_required(VERSION 3.8)
project(clspv_test)

find_package(Vulkan)

add_library(deps STATIC ${PROJECT_SOURCE_DIR}/deps/load_png/lodepng.cpp)
target_compile_features(deps PRIVATE cxx_std_11)
target_include_directories(deps PUBLIC ${PROJECT_SOURCE_DIR}/deps/load_png)

# main
add_executable(main ${PROJECT_SOURCE_DIR}/src/main.cc)
target_include_directories(main PRIVATE ${Vulkan_INCLUDE_DIR})
target_link_libraries(main PRIVATE deps ${Vulkan_LIBRARY})

# fast
add_executable(fast ${PROJECT_SOURCE_DIR}/src/fast.cc)
target_include_directories(fast PRIVATE ${Vulkan_INCLUDE_DIR})
target_link_libraries(fast PRIVATE deps ${Vulkan_LIBRARY})
