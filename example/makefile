SRC=$(wildcard *.cpp)
TAR=$(basename $(SRC))

%:%.cpp
	g++ -o $@ $< -I ../.. -DXPACK_SUPPORT_CHAR_ARRAY
	@echo ============ run $@ ================
	@./$@
	@-rm $@

tar:$(TAR)
	@echo -------test done-------
