ifeq ($(GPP),)
GPP=g++
endif

SRC=$(wildcard *.cpp)
TAR=$(basename $(SRC))

%:%.cpp
	$(GPP) -o $@ $< -Wall -Wextra -I ../.. -DXPACK_SUPPORT_CHAR_ARRAY
	@echo ============ run $@ ================
	@./$@
	@-rm $@

tar:$(TAR)
	@echo -------test done-------
