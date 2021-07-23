ifeq ($(GPP),)
GPP=g++
endif

SRC=$(wildcard *.cpp)
TAR=$(basename $(SRC))

%:%.cpp
	$(GPP) -o $@ -g $< -Wall -Wextra -I ../.. -DXPACK_SUPPORT_CHAR_ARRAY -std=c++11
	@echo ============ run $@ ================
	@./$@
	@-rm $@

tar:$(TAR)
	@echo -------test done-------
