# 指定すべき変数
# CC
# CCFLAGS  
# CXX
# CXXFLAGS  
#
# ALL_O
# ALL_H

ALL_D := $(patsubst %.o,%.d,$(ALL_O))

.PHONY: dummy
dummy: $(ALL_O)

.././obj/%.o: ../%
	mkdir -p $(dir $@)
	@echo TARGET: $@
	@echo DEPS  : $^
	if [ "$(suffix $<)" = ".c" ]; then $(CC) $(CCFLAGS) -c $< -o $@ ; else  $(CXX) $(CXXFLAGS) -c $< -o $@; fi 

.././obj/%.d: ../% $(ALL_H) # .././obj/%.d は 対応する.c or .cc ファイルとすべてのヘッダファイルに依存する
	mkdir -p $(dir $@)
	if [ "$(suffix $<)" = ".c" ]; then $(CC) -MM $< -MF $@.tmp; else  $(CXX) -MM $< -MF $@.tmp; fi 
# オブジェクトファイルのディレクトリを修正 区切りにスラッシュ/があるので区切りに#を用いる
	cat $@.tmp | sed "s#^#$(dir $@)#g" >  $@.tmp.tmp
# 拡張子を修正する
	cat $@.tmp.tmp | sed "s/.o:/$(suffix $<).o:/g" >  $@
	rm $@.tmp $@.tmp.tmp

# ハイフンをつけると(-includeと書くと)エラーを出さない
-include $(ALL_D)
# makeが実行された最初に以下のことが行われる
# .././obj/%.d:... が実行される(更新の必要性があれば) -> includeで*.dファイルが読まれる -> .././obj/%.o:... の依存が*.dファイルに従って上書きされる

