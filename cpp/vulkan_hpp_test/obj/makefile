# 指定すべき変数
# CC
# CCFLAGS  
# CXX
# CXXFLAGS  
#
# INCLUDE_DIR
# THIRD_PARTY_INCLUDE_DIR
#
# ALL_O
# ALL_H
#
# BUILD_TYPE

ALL_D := $(patsubst %.o,%.d,$(ALL_O))
INCLUDE_DIR_ARG := $(addprefix -I,$(INCLUDE_DIR))
THIRD_PARTY_INCLUDE_DIR_ARG := $(addprefix -isystem,$(THIRD_PARTY_INCLUDE_DIR))

COMPILE_COMMON := $(INCLUDE_DIR_ARG) $(THIRD_PARTY_INCLUDE_DIR_ARG)

.PHONY: dummy
dummy: $(ALL_O)

.././obj/$(BUILD_TYPE)/%.o: ../%
	mkdir -p $(dir $@)
	@echo TARGET: $@
	@echo DEPS  : $^
	if [ "$(suffix $<)" = ".c" ]; then $(CC) $(COMPILE_COMMON) $(CCFLAGS) -c $< -o $@ ; else  $(CXX) $(COMPILE_COMMON) $(CXXFLAGS) -c $< -o $@; fi 

.././obj/$(BUILD_TYPE)/%.d: ../% $(ALL_H) # .././obj/$(BUILD_TYPE)/%.d は 対応する.c or .cc ファイルとすべてのヘッダファイルに依存する
	mkdir -p $(dir $@)
	if [ "$(suffix $<)" = ".c" ]; then $(CC) $(COMPILE_COMMON) -MM $< -MF $@.tmp; else  $(CXX) $(COMPILE_COMMON) -MM $< -MF $@.tmp; fi 
# オブジェクトファイルのディレクトリを修正.バックスラッシュを消して一行にしてから処理. 区切りにスラッシュ/があるので区切りに#を用いる
	cat $@.tmp | sed "s/\\\\//g" | tr '\n' ' ' | sed "s#^#$(dir $@)#g" >  $@.tmp.tmp
# 拡張子を修正する
	cat $@.tmp.tmp | sed "s/.o:/$(suffix $<).o:/g" >  $@
	rm $@.tmp $@.tmp.tmp

# ハイフンをつけると(-includeと書くと)エラーを出さない
-include $(ALL_D)
# makeが実行された最初に以下のことが行われる
# .././obj/$(BUILD_TYPE)/%.d:... が実行される(更新の必要性があれば) -> includeで*.dファイルが読まれる -> .././obj/$(BUILD_TYPE)/%.o:... の依存が*.dファイルに従って上書きされる

