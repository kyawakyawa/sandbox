CC     := clang
CCFLAGS := -Wall -Wextra # -Weverything
CCFLAGS_RELEASE := -std=c11 -O3 -DNDEBUG
CCFLAGS_DEBUG := -std=c11 -O0 -g
CXX     := clang++
CXXFLAGS := -Wall -Wextra # -Weverything
CXXFLAGS_RELEASE := -std=c++17 -O3 -DNDEBUG
CXXFLAGS_DEBUG := -std=c++17 -O0 -g

LINKER := lld

INCLUDE_DIR := ./inc ./src
THIRD_PARTY_INCLUDE_DIR := third_party/hoge/include

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

.PHONY: release debug all clean
release: ./main-$(BUILD_TYPE_RELEASE)
$(ALL_O_RELEASE): $(ALL_C_CC_H)
	@echo $(ALL_C)
	@echo $(ALL_CC)
	@echo $(ALL_O_RELEASE)
	@echo $(ALL_H)
	cd obj && $(MAKE) $(COMMON_ARGS) \
		                "CCFLAGS=$(CCFLAGS)  $(CCFLAGS_RELEASE)" "CXXFLAGS=$(CXXFLAGS) $(CXXFLAGS_RELEASE)" \
										"ALL_O=$(addprefix ../,$(ALL_O_RELEASE))" \
										"BUILD_TYPE=$(BUILD_TYPE_RELEASE)"

./obj/$(BUILD_TYPE_RELEASE)/main.o:./main.cc
	mkdir -p ./obj/$(BUILD_TYPE_RELEASE)
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_RELEASE) ./main.cc -o obj/$(BUILD_TYPE_RELEASE)/main.o -c $(addprefix -I,$(MAIN_INCLUDE_DIR))
./main-$(BUILD_TYPE_RELEASE): ./obj/$(BUILD_TYPE_RELEASE)/main.o $(ALL_O_RELEASE)
	$(CXX) -fuse-ld=$(LINKER) $^ -o $@ -lvulkan

debug: ./main-$(BUILD_TYPE_DEBUG)
$(ALL_O_DEBUG): $(ALL_C_CC_H)
	@echo $(ALL_C)
	@echo $(ALL_CC)
	@echo $(ALL_O_DEBUG)
	@echo $(ALL_H)
	cd obj && $(MAKE) $(COMMON_ARGS) \
		                "CCFLAGS=$(CCFLAGS)  $(CCFLAGS_DEBUG)" "CXXFLAGS=$(CXXFLAGS) $(CXXFLAGS_DEBUG)" \
										"ALL_O=$(addprefix ../,$(ALL_O_DEBUG))" \
										"BUILD_TYPE=$(BUILD_TYPE_DEBUG)"

./obj/$(BUILD_TYPE_DEBUG)/main.o:./main.cc
	mkdir -p ./obj/$(BUILD_TYPE_DEBUG)
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_DEBUG) ./main.cc -o obj/$(BUILD_TYPE_DEBUG)/main.o -c $(addprefix -I,$(MAIN_INCLUDE_DIR))
./main-$(BUILD_TYPE_DEBUG): ./obj/$(BUILD_TYPE_DEBUG)/main.o $(ALL_O_DEBUG)
	$(CXX) -fuse-ld=$(LINKER) $^ -o $@ -lvulkan

all: release debug

clean:
	$(RM) -r $(addprefix ./obj/, $(BUILD_TYPES))
