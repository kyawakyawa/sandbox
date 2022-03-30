cmake_minimum_required(VERSION 3.8)
project(clspv_test)

find_package(Vulkan)

add_library(deps STATIC ${PROJECT_SOURCE_DIR}/deps/load_png/lodepng.cpp)
target_compile_features(deps PRIVATE cxx_std_11)
target_include_directories(deps PUBLIC ${PROJECT_SOURCE_DIR}/deps/load_png)

set(TARGETS "")

# main
add_executable(main ${PROJECT_SOURCE_DIR}/src/main.cc)
list(APPEND TARGETS main)

# fast
add_executable(fast ${PROJECT_SOURCE_DIR}/src/fast.cc)
list(APPEND TARGETS fast)

# gaussian filter
add_executable(gaussian_filter ${PROJECT_SOURCE_DIR}/src/gaussian_filter.cc)
list(APPEND TARGETS gaussian_filter)

foreach(TARGET IN LISTS TARGETS)
  # Vulkan
  target_include_directories(${TARGET} PRIVATE ${Vulkan_INCLUDE_DIR})
  target_link_libraries(${TARGET} PRIVATE deps ${Vulkan_LIBRARY})
endforeach()
