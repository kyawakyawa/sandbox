cmake_minimum_required(VERSION 3.10)
project(spdlog-test VERSION 0.0.1)

find_package(spdlog  CONFIG REQUIRED)

add_library(spdlog-test-lib SHARED lib/lib.cc)
add_executable(spdlog-test main.cc)

target_include_directories(spdlog-test-lib PUBLIC ${PROJECT_SOURCE_DIR}/lib)
target_link_libraries(spdlog-test-lib PRIVATE spdlog::spdlog)
target_link_libraries(spdlog-test PRIVATE spdlog-test-lib spdlog::spdlog)

target_compile_features(spdlog-test-lib PRIVATE cxx_std_17)
target_compile_features(spdlog-test PRIVATE cxx_std_17)
