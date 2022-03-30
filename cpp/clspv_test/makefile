# MIT License
# 
# Copyright (c) 2022 kyawakyawa
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

CC := clang
CXX := clang++

CMAKE_BUILD_RELEASE_DIR := cmake-build-release
CMAKE_BUILD_DEBUG_DIR := cmake-build-debug

ALL_C_CL := $(wildcard ./opencl/c/*.cl)
ALL_C_SPIRV  := $(patsubst ./opencl/c/%.cl,./spirv/c/%.spv,$(ALL_C_CL)) 
ALL_C_CSV  := $(patsubst ./opencl/c/%.cl,./spirv/c/%.csv,$(ALL_C_CL)) 
ALL_C_TXT  := $(patsubst ./opencl/c/%.cl,./spirv/c/%.txt,$(ALL_C_CL)) 
ALL_C_JSON  := $(patsubst ./opencl/c/%.cl,./spirv/c/%.json,$(ALL_C_CL)) 
ALL_C_HLSL  := $(patsubst ./opencl/c/%.cl,./spirv/c/%.hlsl,$(ALL_C_CL)) 

PHONY_BASE := all release debug common cmake_configure_release cmake_configure_debug cmake_build_release cmake_build_debug weak_clean clean
PHONY_OPENCL := opencl
.PHONY: $(PHONY_BASE) $(PHONY_OPENCL)

release: cmake_configure_release cmake_build_release common
debug: cmake_configure_debug cmake_build_debug common

all: release debug

common: opencl

# CMake Release Configure
cmake_configure_release: $(CMAKE_BUILD_RELEASE_DIR)/Makefile
$(CMAKE_BUILD_RELEASE_DIR)/Makefile:
	CC=$(CC) CXX=$(CXX) cmake -H. -B$(CMAKE_BUILD_RELEASE_DIR) -DCMAKE_BUILD_TYPE=Release

# CMake Debug Configure
cmake_configure_debug: $(CMAKE_BUILD_DEBUG_DIR)/Makefile
$(CMAKE_BUILD_DEBUG_DIR)/Makefile:
	CC=$(CC) CXX=$(CXX) cmake -H. -B$(CMAKE_BUILD_DEBUG_DIR) -DCMAKE_BUILD_TYPE=Debug

# CMake Release Build
cmake_build_release: cmake_configure_release
	cd $(CMAKE_BUILD_RELEASE_DIR) && $(MAKE)

# CMake Debug Build
cmake_build_debug: cmake_configure_debug
	cd $(CMAKE_BUILD_DEBUG_DIR) && $(MAKE)

### OpenCL #################################################################

opencl: $(ALL_C_SPIRV) $(ALL_C_CSV) $(ALL_C_TXT) $(ALL_C_JSON) $(ALL_C_HLSL)

./spirv/c/%.spv:./opencl/c/%.cl
	clang -Xclang -finclude-default-header -xcl -cl-std=CL2.0 -fsyntax-only -Wall -Wextra $<
	clspv -o=$@ $< -O=3 -w

./spirv/c/%.csv:./spirv/c/%.spv
	clspv-reflection -o $@ $<

./spirv/c/%.txt:./spirv/c/%.spv
	spirv-dis -o $@ $<

./spirv/c/%.json:./spirv/c/%.spv
	spirv-cross $< --output $@ --reflect

./spirv/c/%.hlsl:./spirv/c/%.spv
	spirv-cross $< --output $@ --hlsl --shader-model 62 --hlsl-enable-16bit-types

############################################################################

weak_clean:
# CMake
	if [ -f $(CMAKE_BUILD_RELEASE_DIR)/Makefile ]; then cd $(CMAKE_BUILD_RELEASE_DIR) && $(MAKE) clean && cd .. ; fi
	if [ -f $(CMAKE_BUILD_DEBUG_DIR)/Makefile ]; then cd $(CMAKE_BUILD_DEBUG_DIR) && $(MAKE) clean && cd .. ; fi

# OpenCL
	$(RM) $(ALL_C_SPIRV) $(ALL_C_CSV) $(ALL_C_TXT) $(ALL_C_JSON) $(ALL_C_HLSL)

# if [ -f $(CMAKE_BUILD_RELEASE_DIR)/Makefile ]; then cmake --build $(CMAKE_BUILD_RELEASE_DIR) --target clean ; fi
# if [ -f $(CMAKE_BUILD_DEBUG_DIR)/Makefile ]; then cmake --build $(CMAKE_BUILD_DEBUG_DIR) --target clean ; fi

clean: weak_clean
	$(RM) -r $(CMAKE_BUILD_RELEASE_DIR) $(CMAKE_BUILD_DEBUG_DIR)
