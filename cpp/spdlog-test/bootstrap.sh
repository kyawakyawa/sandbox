#!/bin/bash


CC=clang CXX=clang++ cmake \
  -Bcmake-build-debug -H. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=YES \
  -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-linux-release \
  -DVCPKG_MANIFEST_MODE=ON
