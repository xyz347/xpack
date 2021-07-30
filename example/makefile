ifeq ($(GPP),)
GPP=g++
endif

ifneq ($(c11),off)
CXXFLAG=-std=c++11
endif

SRC=$(wildcard *.cpp)
TAR=$(basename $(SRC))

%:%.cpp
	$(GPP) -o $@ -g $< -Wall -Wextra -I ../.. ${CXXFLAG}
	@echo ============ run $@ ================
	@./$@
	@-rm $@
	@-rm -rf $@.dSYM

tar:$(TAR)
	@echo -------test done-------
