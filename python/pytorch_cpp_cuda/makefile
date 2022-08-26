MODULE_NAME = lltm
COMMON_DEF = $(shell echo '-DTORCH_API_INCLUDE_EXTENSION_H -DPYBIND11_COMPILER_TYPE=\\\"_clang\\\" -DPYBIND11_STDLIB=\\\"_libstdcpp\\\" -DPYBIND11_BUILD_ABI=\\\"_cxxabi1011\\\" -D_GLIBCXX_USE_CXX11_ABI=0')

CC     := clang-15
CCFLAGS := -Wall -Wextra -fPIC # -Weverything
CCFLAGS_RELEASE := -std=c11 -O3 -DNDEBUG
CCFLAGS_DEBUG := -std=c11 -O0 -g
CXX     := clang++-15
CXXFLAGS := -Wall -Wextra -fPIC -DTORCH_EXTENSION_NAME=$(MODULE_NAME) $(COMMON_DEF)# -Weverything
CXXFLAGS_RELEASE := -std=c++17 -O3 -DNDEBUG
CXXFLAGS_DEBUG := -std=c++17 -O0 -g
CUDAC     := clang++-15
CUDACFLAGS := -Wall -Wextra -fPIC -xcuda -std=c++14 --cuda-gpu-arch=sm_61 -D__CUDA_NO_HALF_OPERATORS__ -D__CUDA_NO_HALF_CONVERSIONS__ -D__CUDA_NO_BFLOAT16_CONVERSIONS__ -D__CUDA_NO_HALF2_OPERATORS__ $(COMMON_DEF)
CUDACFLAGS_RELEASE := -O3 -DNDEBUG 
CUDACFLAGS_DEBUG := -O0 -g

LINKER := lld-15

INCLUDE_DIR := ./inc ./src
THIRD_PARTY_INCLUDE_DIR := $(shell python -c "import torch,os.path;print(os.path.join(torch.__path__[0],'include'))" | xargs -n1 realpath --relative-to=.) \
													 $(shell python -c "import torch,os.path;print(os.path.join(torch.__path__[0],'include/torch/csrc/api/include'))" | xargs -n1 realpath --relative-to=.) \
													 $(shell python -c "import torch,os.path;print(os.path.join(torch.__path__[0],'include/TH'))" | xargs -n1 realpath --relative-to=.) \
													 $(shell python -c "import torch,os.path;print(os.path.join(torch.__path__[0],'include/THC'))" | xargs -n1 realpath --relative-to=.) \
													 $(shell echo $(dirname $(which nvcc))/../include | xargs -n1 realpath --relative-to=.) \
													 $(shell python -c "from sysconfig import get_paths as gp; print(gp()['include'])" | xargs -n1 realpath --relative-to=.)

MAIN_INCLUDE_DIR := ./inc

SRC_ROOT_DIR := ./src
SRC_DIR := $(SRC_ROOT_DIR) $(SRC_ROOT_DIR)/*

ALL_C := $(wildcard $(patsubst %,%/*.c,$(SRC_DIR)))
ALL_CC := $(wildcard $(patsubst %,%/*.cc,$(SRC_DIR)))
ALL_CU := $(wildcard $(patsubst %,%/*.cu,$(SRC_DIR)))

ALL_C_CC_CU := $(ALL_C) $(ALL_CC) $(ALL_CU)

BUILD_TYPE_RELEASE := release
BUILD_TYPE_DEBUG := debug

BUILD_TYPES := $(BUILD_TYPE_RELEASE) $(BUILD_TYPE_DEBUG)

ALL_O_RELEASE := $(addprefix ./obj/$(BUILD_TYPE_RELEASE)/,$(addsuffix .o,$(ALL_C_CC_CU)))
ALL_O_DEBUG := $(addprefix ./obj/$(BUILD_TYPE_DEBUG)/,$(addsuffix .o,$(ALL_C_CC_CU)))

ALL_H := $(wildcard inc/*.h inc/*/*.h $(SRC_ROOT_DIR)/*.h $(SRC_ROOT_DIR)/*/*.h)

ALL_C_CC_CU_H := $(ALL_C_CC_CU) $(ALL_H)

COMMON_ARGS := "CC=$(CC)" "CXX=$(CXX)" \
							 "ALL_H=$(addprefix ../,$(ALL_H))" \
							 "INCLUDE_DIR=$(addprefix ../,$(INCLUDE_DIR))" \
							 "THIRD_PARTY_INCLUDE_DIR=$(addprefix ../,$(THIRD_PARTY_INCLUDE_DIR))"

TORCH_LIB_PATH=$(shell python -c "import torch,os.path;print(os.path.join(torch.__path__[0],'lib'))")
PYTHON_LIB_PATH=$(shell python -c "from sysconfig import get_paths as gp; import os.path; print(os.path.join(gp()['scripts'],'../lib/'))" )

OUT_SO := $(MODULE_NAME)$(shell python3-config --extension-suffix)
OUT_RELEASE_SO := $(MODULE_NAME)-$(BUILD_TYPE_RELEASE)$(shell python3-config --extension-suffix)
OUT_DEBUG_SO := $(MODULE_NAME)-$(BUILD_TYPE_DEBUG)$(shell python3-config --extension-suffix)

.PHONY: release release_obj debug debug_obj all clean
release: $(OUT_RELEASE_SO)
	if [ -e $(OUT_SO) ]; then unlink $(OUT_SO) ; fi
	ln -s $(OUT_RELEASE_SO) $(OUT_SO)
	$(MAKE) create_stub_files

release_obj: $(ALL_C_CC_CU_H)
	@echo $(ALL_C)
	@echo $(ALL_CC)
	@echo $(ALL_CU)
	@echo $(ALL_O_RELEASE)
	@echo $(ALL_H)
	cd obj && $(MAKE) $(COMMON_ARGS) \
		                "CCFLAGS=$(CCFLAGS)  $(CCFLAGS_RELEASE)" "CXXFLAGS=$(CXXFLAGS) $(CXXFLAGS_RELEASE)" \
		                "CUDAC=$(CUDAC)" \
		                "CUDACFLAGS=$(CUDACFLAGS)  $(CUDACFLAGS_RELEASE)" \
										"ALL_O=$(addprefix ../,$(ALL_O_RELEASE))" \
										"BUILD_TYPE=$(BUILD_TYPE_RELEASE)"

$(OUT_RELEASE_SO): release_obj
	$(CXX) -fuse-ld=$(LINKER) $(ALL_O_RELEASE) $(LINK_FLAGS_PYBIND11) -shared -o $@ -Wl,-L$(PYTHON_LIB_PATH) -L$(TORCH_LIB_PATH) -Wl,-rpath,$(PYTHON_LIB_PATH) -lc10 -ltorch_cpu -ltorch_python -lcudart -lstdc++

# ./obj/$(BUILD_TYPE_RELEASE)/main.o:./main.cc
# 	mkdir -p ./obj/$(BUILD_TYPE_RELEASE)
# 	$(CXX) $(CXXFLAGS) $(CXXFLAGS_RELEASE) ./main.cc -o obj/$(BUILD_TYPE_RELEASE)/main.o -c $(addprefix -I,$(MAIN_INCLUDE_DIR))
# ./main-$(BUILD_TYPE_RELEASE): ./obj/$(BUILD_TYPE_RELEASE)/main.o release_obj
# 	$(CXX) -fuse-ld=$(LINKER) $(ALL_O_RELEASE) $< -o $@

debug: $(OUT_DEBUG_SO)
	if [ -e $(OUT_SO) ]; then unlink $(OUT_SO) ; fi
	ln -s $(OUT_DEBUG_SO) $(OUT_SO)
	$(MAKE) create_stub_files

debug_obj: $(ALL_C_CC_CU_H)
	@echo $(ALL_C)
	@echo $(ALL_CC)
	@echo $(ALL_CU)
	@echo $(ALL_O_DEBUG)
	@echo $(ALL_H)
	cd obj && $(MAKE) $(COMMON_ARGS) \
		                "CCFLAGS=$(CCFLAGS)  $(CCFLAGS_DEBUG)" "CXXFLAGS=$(CXXFLAGS) $(CXXFLAGS_DEBUG)" \
		                "CUDAC=$(CUDAC)" \
		                "CUDACFLAGS=$(CUDACFLAGS)  $(CUDACFLAGS_DEBUG)" \
										"ALL_O=$(addprefix ../,$(ALL_O_DEBUG))" \
										"BUILD_TYPE=$(BUILD_TYPE_DEBUG)"

$(OUT_DEBUG_SO): debug_obj
	$(CXX) -fuse-ld=$(LINKER) $(ALL_O_DEBUG) $(LINK_FLAGS_PYBIND11) -shared -o $@ -Wl,-L$(PYTHON_LIB_PATH) -L$(TORCH_LIB_PATH) -Wl,-rpath,$(PYTHON_LIB_PATH) -lc10 -ltorch_cpu -ltorch_python -lcudart -lstdc++

# ./obj/$(BUILD_TYPE_DEBUG)/main.o:./main.cc
# 	mkdir -p ./obj/$(BUILD_TYPE_DEBUG)
# 	$(CXX) $(CXXFLAGS) $(CXXFLAGS_DEBUG) ./main.cc -o obj/$(BUILD_TYPE_DEBUG)/main.o -c $(addprefix -I,$(MAIN_INCLUDE_DIR))
# ./main-$(BUILD_TYPE_DEBUG): ./obj/$(BUILD_TYPE_DEBUG)/main.o debug_obj
# 	$(CXX) -fuse-ld=$(LINKER) $(ALL_O_DEBUG) $< -o $@

all: release debug

create_stub_files:
	LD_LIBRARY_PATH=$(TORCH_LIB_PATH):$(PYTHON_LIB_PATH) PYTHONPATH=. pybind11-stubgen $(MODULE_NAME) --no-setup-py --root-module-suffix="" --ignore-invalid=all --output-dir="./typings" --skip-signature-downgrade

clean:
	$(RM) -r $(addprefix ./obj/, $(BUILD_TYPES))
