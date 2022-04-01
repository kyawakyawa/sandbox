CC     := clang
CCFLAGS := -Wall -Wextra # -Weverything
CCFLAGS_RELEASE := -std=c11 -O3 -DNDEBUG
CCFLAGS_DEBUG := -std=c++11 -O0 -g
CXX     := clang++
CXXFLAGS := -Wall -Wextra # -Weverything
CXXFLAGS_RELEASE := -std=c++17 -O3 -DNDEBUG
CXXFLAGS_DEBUG := -std=c++17 -O0 -g

SRC_ROOT_DIR := ./src
SRC_DIR := $(SRC_ROOT_DIR) $(SRC_ROOT_DIR)/*

ALL_C := $(wildcard $(patsubst %,%/*.c,$(SRC_DIR)))
ALL_CC := $(wildcard $(patsubst %,%/*.cc,$(SRC_DIR)))

ALL_O := $(addprefix ./obj/,$(addsuffix .o,$(ALL_C) $(ALL_CC)))

ALL_H := $(wildcard inc/*.h inc/*/*.h $(SRC_ROOT_DIR)/*.h $(SRC_ROOT_DIR)/*/*.h)

.PHONY: release debug clean
release:
	@echo $(ALL_C)
	@echo $(ALL_CC)
	@echo $(ALL_O)
	@echo $(ALL_H)
	cd obj && $(MAKE) "CC=$(CC)" "CCFLAGS=$(CCFLAGS) $(CCFLAGS_RELEASE)" "CXX=$(CXX)" "CXXFLAGS=$(CXXFLAGS) $(CXXFLAGS_RELEASE)" "ALL_O=$(addprefix ../,$(ALL_O))" "ALL_H=$(addprefix ../,$(ALL_H))"

clean:
	$(RM) -r obj/$(SRC_ROOT_DIR)
