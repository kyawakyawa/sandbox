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

BINDING_INCLUDE_DIR := ./inc

SRC_ROOT_DIR := ./src
SRC_DIR := $(SRC_ROOT_DIR) $(SRC_ROOT_DIR)/*

ALL_C := $(wildcard $(patsubst %,%/*.c,$(SRC_DIR)))
ALL_CC := $(wildcard $(patsubst %,%/*.cc,$(SRC_DIR)))
BINDING_CC := ./hoge.cc

ALL_C_CC := $(ALL_C) $(ALL_CC)

BUILD_TYPE_RELEASE := release
BUILD_TYPE_DEBUG := debug

BUILD_TYPES := $(BUILD_TYPE_RELEASE) $(BUILD_TYPE_DEBUG)

ALL_O_RELEASE := $(addprefix ./obj/$(BUILD_TYPE_RELEASE)/,$(addsuffix .o,$(ALL_C_CC)))
ALL_O_DEBUG := $(addprefix ./obj/$(BUILD_TYPE_DEBUG)/,$(addsuffix .o,$(ALL_C_CC)))

BINDING_O_RELEASE := $(addprefix ./obj/${BUILD_TYPE_RELEASE}/, $(addsuffix .o, $(BINDING_CC)))
BINDING_O_DEBUG := $(addprefix ./obj/${BUILD_TYPE_DEBUG}/, $(addsuffix .o, $(BINDING_CC)))

BINDING_D_RELEASE := $(patsubst %.o,%.d, $(BINDING_O_RELEASE))
BINDING_D_DEBUG := $(patsubst %.o,%.d, $(BINDING_O_DEBUG))

BINDING_INCLUDE_DIR_ARG := $(addprefix -I,$(BINDING_INCLUDE_DIR)) $(addprefix -isystem,$(THIRD_PARTY_INCLUDE_DIR))

ALL_H := $(wildcard inc/*.h inc/*/*.h $(SRC_ROOT_DIR)/*.h $(SRC_ROOT_DIR)/*/*.h)

ALL_C_CC_H := $(ALL_C_CC) $(ALL_H)

COMMON_ARGS := "CC=$(CC)" "CXX=$(CXX)" \
							 "ALL_H=$(addprefix ../,$(ALL_H))" \
							 "INCLUDE_DIR=$(addprefix ../,$(INCLUDE_DIR))" \
							 "THIRD_PARTY_INCLUDE_DIR=$(addprefix ../,$(THIRD_PARTY_INCLUDE_DIR))"

OUT_SO := hoge$(shell python3-config --extension-suffix)
OUT_RELEASE_SO := hoge-$(BUILD_TYPE_RELEASE)$(shell python3-config --extension-suffix)
OUT_DEBUG_SO := hoge-$(BUILD_TYPE_DEBUG)$(shell python3-config --extension-suffix)

ifeq  ($(shell uname),Darwin) # MACのとき

LINK_FLAGS_PYBIND11 := -undefined dynamic_lookup

else

LINK_FLAGS_PYBIND11 := ""

endif

.PHONY: release release_obj debug debug_obj create_stub_files clean
release: $(OUT_RELEASE_SO)
	if [ -e $(OUT_SO) ]; then unlink $(OUT_SO) ; fi
	ln -s $(OUT_RELEASE_SO) $(OUT_SO)
	$(MAKE) create_stub_files

release_obj: $(ALL_C_CC_H)
	@echo $(ALL_C)
	@echo $(ALL_CC)
	@echo $(ALL_O_RELEASE)
	@echo $(ALL_H)
	cd obj && $(MAKE) $(COMMON_ARGS) \
		                "CCFLAGS=$(CCFLAGS)  $(CCFLAGS_RELEASE)" "CXXFLAGS=$(CXXFLAGS) $(CXXFLAGS_RELEASE)" \
										"ALL_O=$(addprefix ../,$(ALL_O_RELEASE))" \
										"BUILD_TYPE=$(BUILD_TYPE_RELEASE)"

./obj/$(BUILD_TYPE_RELEASE)/%.o:%
	$(CXX) $(BINDING_INCLUDE_DIR_ARG) $(CXXFLAGS) -c $< -o $@

$(OUT_RELEASE_SO):$(BINDING_O_RELEASE) release_obj
	$(CXX) -fuse-ld=$(LINKER) $(ALL_O_RELEASE) $(BINDING_O_RELEASE) $(LINK_FLAGS_PYBIND11) -shared -o $@

debug: $(OUT_DEBUG_SO)
	if [ -e $(OUT_SO) ]; then unlink $(OUT_SO) ; fi
	ln -s $(OUT_DEBUG_SO) $(OUT_SO)
	$(MAKE) create_stub_files

debug_obj: $(ALL_C_CC_H)
	@echo $(ALL_C)
	@echo $(ALL_CC)
	@echo $(ALL_O_DEBUG)
	@echo $(ALL_H)
	cd obj && $(MAKE) $(COMMON_ARGS) \
		                "CCFLAGS=$(CCFLAGS)  $(CCFLAGS_DEBUG)" "CXXFLAGS=$(CXXFLAGS) $(CXXFLAGS_DEBUG)" \
										"ALL_O=$(addprefix ../,$(ALL_O_DEBUG))" \
										"BUILD_TYPE=$(BUILD_TYPE_DEBUG)"

./obj/$(BUILD_TYPE_DEBUG)/%.o:%
	$(CXX) $(BINDING_INCLUDE_DIR_ARG) $(CXXFLAGS) -c $< -o $@

$(OUT_DEBUG_SO):$(BINDING_O_DEBUG) debug_obj
	$(CXX) -fuse-ld=$(LINKER) $(ALL_O_DEBUG) $(BINDING_O_DEBUG) $(LINK_FLAGS_PYBIND11) -shared -o $@

create_stub_files:
	PYTHONPATH=. pybind11-stubgen hoge --no-setup-py --root-module-suffix="" --ignore-invalid=all --output-dir="./typings"

clean:
	if [ -e $(OUT_SO) ]; then unlink $(OUT_SO) ; fi
	$(RM) $(OUT_RELEASE_SO) $(OUT_DEBUG_SO)
	$(RM) -r $(addprefix ./obj/, $(BUILD_TYPES))

./obj/$(BUILD_TYPE_RELEASE)/%.d:% $(BINDING_INCLUDE_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(BINDING_INCLUDE_DIR_ARG) -MM $< -MF $@.tmp
# オブジェクトファイルのディレクトリを修正.バックスラッシュを消して一行にしてから処理. 区切りにスラッシュ/があるので区切りに#を用いる
	cat $@.tmp | sed "s/\\\\//g" | tr '\n' ' ' | sed "s#^#$(dir $@)#g" >  $@.tmp.tmp
# 拡張子を修正する
	cat $@.tmp.tmp | sed "s/.o:/$(suffix $<).o:/g" >  $@
	rm $@.tmp $@.tmp.tmp

./obj/$(BUILD_TYPE_DEBUG)/%.d:% $(BINDING_INCLUDE_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(BINDING_INCLUDE_DIR_ARG)  -MM $< -MF $@.tmp
# オブジェクトファイルのディレクトリを修正.バックスラッシュを消して一行にしてから処理. 区切りにスラッシュ/があるので区切りに#を用いる
	cat $@.tmp | sed "s/\\\\//g" | tr '\n' ' ' | sed "s#^#$(dir $@)#g" >  $@.tmp.tmp
# 拡張子を修正する
	cat $@.tmp.tmp | sed "s/.o:/$(suffix $<).o:/g" >  $@
	rm $@.tmp $@.tmp.tmp

-include $(BINDING_D_RELEASE)
-include $(BINDING_D_DEBUG)
