CC     := clang
CCFLAGS := -Wall -Wextra # -Weverything
CCFLAGS_RELEASE := -std=c11 -O3 -DNDEBUG
CCFLAGS_DEBUG := -std=c11 -O0 -g
CXX     := clang++
CXXFLAGS := -Wall -Wextra # -Weverything
CXXFLAGS_RELEASE := -std=c++17 -O3 -DNDEBUG
CXXFLAGS_DEBUG := -std=c++17 -O0 -g

SRC_ROOT_DIR := ./src
SRC_DIR := $(SRC_ROOT_DIR) $(SRC_ROOT_DIR)/*

ALL_C := $(wildcard $(patsubst %,%/*.c,$(SRC_DIR)))
ALL_CC := $(wildcard $(patsubst %,%/*.cc,$(SRC_DIR)))

BUILD_TYPE_RELEASE := release
BUILD_TYPE_DEBUG := debug

BUILD_TYPES := $(BUILD_TYPE_RELEASE) $(BUILD_TYPE_DEBUG)

ALL_O_RELEASE := $(addprefix ./obj/$(BUILD_TYPE_RELEASE)/,$(addsuffix .o,$(ALL_C) $(ALL_CC)))
ALL_O_DEBUG := $(addprefix ./obj/$(BUILD_TYPE_DEBUG)/,$(addsuffix .o,$(ALL_C) $(ALL_CC)))

ALL_H := $(wildcard inc/*.h inc/*/*.h $(SRC_ROOT_DIR)/*.h $(SRC_ROOT_DIR)/*/*.h)

.PHONY: release debug all clean
release:
	@echo $(ALL_C)
	@echo $(ALL_CC)
	@echo $(ALL_O_RELEASE)
	@echo $(ALL_H)
	cd obj && $(MAKE) "CC=$(CC)" "CCFLAGS=$(CCFLAGS) $(CCFLAGS_RELEASE)" "CXX=$(CXX)" "CXXFLAGS=$(CXXFLAGS) $(CXXFLAGS_RELEASE)" "ALL_O=$(addprefix ../,$(ALL_O_RELEASE))" "ALL_H=$(addprefix ../,$(ALL_H))" "BUILD_TYPE=$(BUILD_TYPE_RELEASE)"

debug:
	@echo $(ALL_C)
	@echo $(ALL_CC)
	@echo $(ALL_O_DEBUG)
	@echo $(ALL_H)
	cd obj && $(MAKE) "CC=$(CC)" "CCFLAGS=$(CCFLAGS) $(CCFLAGS_DEBUG)" "CXX=$(CXX)" "CXXFLAGS=$(CXXFLAGS) $(CXXFLAGS_DEBUG)" "ALL_O=$(addprefix ../,$(ALL_O_DEBUG))" "ALL_H=$(addprefix ../,$(ALL_H))" "BUILD_TYPE=$(BUILD_TYPE_DEBUG)"

all: release debug

clean:
	$(RM) -r $(addprefix ./obj/, $(BUILD_TYPES))
