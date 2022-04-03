CC     := clang
CCFLAGS := -Wall -Wextra -fPIC
CCFLAGS_RELEASE := -std=c11 -O3 -DNDEBUG
CCFLAGS_DEBUG := -std=c11 -O0 -g
CXX     := clang++
CXXFLAGS := -Wall -Wextra -fPIC
CXXFLAGS_RELEASE := -std=c++17 -O3 -DNDEBUG
CXXFLAGS_DEBUG := -std=c++17 -O0 -g

LINKER := lld

INCLUDE_DIR := ./inc ./src
THIRD_PARTY_INCLUDE_DIR := $(patsubst -I%,./%,$(shell python3 -m pybind11 --includes | sed "s/-I//g" | xargs -n1 realpath --relative-to=. | tr '\n' ' '))

MAIN_INCLUDE_DIR := ./inc

SRC_ROOT_DIR := ./src
SRC_DIR := $(SRC_ROOT_DIR) $(SRC_ROOT_DIR)/*

ALL_C := $(wildcard $(patsubst %,%/*.c,$(SRC_DIR)))
ALL_CC := $(wildcard $(patsubst %,%/*.cc,$(SRC_DIR)))

ALL_C_CC := $(ALL_C) $(ALL_CC)

BUILD_TYPE_RELEASE := release
BUILD_TYPE_DEBUG := debug

BUILD_TYPES := $(BUILD_TYPE_RELEASE) $(BUILD_TYPE_DEBUG)

ALL_O_RELEASE := $(addprefix ./obj/$(BUILD_TYPE_RELEASE)/,$(addsuffix .o,$(ALL_C_CC)))
ALL_O_DEBUG := $(addprefix ./obj/$(BUILD_TYPE_DEBUG)/,$(addsuffix .o,$(ALL_C_CC)))

ALL_H := $(wildcard inc/*.h inc/*/*.h $(SRC_ROOT_DIR)/*.h $(SRC_ROOT_DIR)/*/*.h)

ALL_C_CC_H := $(ALL_C_CC) $(ALL_H)

COMMON_ARGS := "CC=$(CC)" "CXX=$(CXX)" \
							 "ALL_H=$(addprefix ../,$(ALL_H))" \
							 "INCLUDE_DIR=$(addprefix ../,$(INCLUDE_DIR))" \
							 "THIRD_PARTY_INCLUDE_DIR=$(addprefix ../,$(THIRD_PARTY_INCLUDE_DIR))"

OUT_SO := hoge$(shell python3-config --extension-suffix)
OUT_RELEASE_SO := hoge-$(BUILD_TYPE_RELEASE)$(shell python3-config --extension-suffix)
OUT_DEBUG_SO := hoge-$(BUILD_TYPE_DEBUG)$(shell python3-config --extension-suffix)

.PHONY: release debug create_stub_files clean
release: $(OUT_RELEASE_SO)
	if [ -e $(OUT_SO) ]; then unlink $(OUT_SO) ; fi
	ln -s $(OUT_RELEASE_SO) $(OUT_SO)

$(ALL_O_RELEASE): $(ALL_C_CC_H)
	@echo $(ALL_C)
	@echo $(ALL_CC)
	@echo $(ALL_O_RELEASE)
	@echo $(ALL_H)
	cd obj && $(MAKE) $(COMMON_ARGS) \
		                "CCFLAGS=$(CCFLAGS)  $(CCFLAGS_RELEASE)" "CXXFLAGS=$(CXXFLAGS) $(CXXFLAGS_RELEASE)" \
										"ALL_O=$(addprefix ../,$(ALL_O_RELEASE))" \
										"BUILD_TYPE=$(BUILD_TYPE_RELEASE)"

$(OUT_RELEASE_SO):$(ALL_O_RELEASE)
	$(CXX) -fuse-ld=$(LINKER) $^ -shared -o $@

debug: $(OUT_DEBUG_SO)
	if [ -e $(OUT_SO) ]; then unlink $(OUT_SO) ; fi
	ln -s $(OUT_DEBUG_SO) $(OUT_SO)

$(ALL_O_DEBUG): $(ALL_C_CC_H)
	@echo $(ALL_C)
	@echo $(ALL_CC)
	@echo $(ALL_O_DEBUG)
	@echo $(ALL_H)
	cd obj && $(MAKE) $(COMMON_ARGS) \
		                "CCFLAGS=$(CCFLAGS)  $(CCFLAGS_DEBUG)" "CXXFLAGS=$(CXXFLAGS) $(CXXFLAGS_DEBUG)" \
										"ALL_O=$(addprefix ../,$(ALL_O_DEBUG))" \
										"BUILD_TYPE=$(BUILD_TYPE_DEBUG)"

$(OUT_DEBUG_SO):$(ALL_O_DEBUG)
	$(CXX) -fuse-ld=$(LINKER) $^ -shared -o $@

create_stub_files:
	PYTHONPATH=. pybind11-stubgen hoge --no-setup-py --root-module-suffix="" --ignore-invalid=all --output-dir="./typings"

clean:
	if [ -e $(OUT_SO) ]; then unlink $(OUT_SO) ; fi
	$(RM) $(OUT_RELEASE_SO) $(OUT_DEBUG_SO)
	$(RM) -r $(addprefix ./obj/, $(BUILD_TYPES))
