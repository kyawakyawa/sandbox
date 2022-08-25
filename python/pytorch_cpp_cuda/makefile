CC     := clang
CCFLAGS := -Wall -Wextra # -Weverything
CCFLAGS_RELEASE := -std=c11 -O3 -DNDEBUG
CCFLAGS_DEBUG := -std=c11 -O0 -g
CXX     := clang++
CXXFLAGS := -Wall -Wextra # -Weverything
CXXFLAGS_RELEASE := -std=c++17 -O3 -DNDEBUG
CXXFLAGS_DEBUG := -std=c++17 -O0 -g
CUDAC     := clang++
CUDACFLAGS := -Wall -Wextra # -Weverything
CUDACFLAGS_RELEASE := -O3 -DNDEBUG -xcuda -std=c++14 --cuda-gpu-arch=sm_50 -D__CUDA_NO_HALF_OPERATORS__ -D__CUDA_NO_HALF_CONVERSIONS__ -D__CUDA_NO_BFLOAT16_CONVERSIONS__ -D__CUDA_NO_HALF2_OPERATORS__ -DTORCH_API_INCLUDE_EXTENSION_H -fsyntax-only -nocudalib #FIXME
CUDACFLAGS_DEBUG := -O0 -g -xcuda -std=cuda --cuda-gpu-arch=sm_50 -nocudalib #FIXME

LINKER := lld

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

.PHONY: release release_obj debug debug_obj all clean
# release: ./main-$(BUILD_TYPE_RELEASE)
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

# ./obj/$(BUILD_TYPE_RELEASE)/main.o:./main.cc
# 	mkdir -p ./obj/$(BUILD_TYPE_RELEASE)
# 	$(CXX) $(CXXFLAGS) $(CXXFLAGS_RELEASE) ./main.cc -o obj/$(BUILD_TYPE_RELEASE)/main.o -c $(addprefix -I,$(MAIN_INCLUDE_DIR))
# ./main-$(BUILD_TYPE_RELEASE): ./obj/$(BUILD_TYPE_RELEASE)/main.o release_obj
# 	$(CXX) -fuse-ld=$(LINKER) $(ALL_O_RELEASE) $< -o $@

# debug: ./main-$(BUILD_TYPE_DEBUG)
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

# ./obj/$(BUILD_TYPE_DEBUG)/main.o:./main.cc
# 	mkdir -p ./obj/$(BUILD_TYPE_DEBUG)
# 	$(CXX) $(CXXFLAGS) $(CXXFLAGS_DEBUG) ./main.cc -o obj/$(BUILD_TYPE_DEBUG)/main.o -c $(addprefix -I,$(MAIN_INCLUDE_DIR))
# ./main-$(BUILD_TYPE_DEBUG): ./obj/$(BUILD_TYPE_DEBUG)/main.o debug_obj
# 	$(CXX) -fuse-ld=$(LINKER) $(ALL_O_DEBUG) $< -o $@

all: release debug

clean:
	$(RM) -r $(addprefix ./obj/, $(BUILD_TYPES))
