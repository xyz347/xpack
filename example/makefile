ifeq ($(GPP),)
GPP=g++
endif

SRC=$(wildcard *.cpp)
TAR=$(basename $(SRC))

ifeq ($(c11),off)
NEEDC11=sharedptr
TAR:=$(filter-out $(NEEDC11),$(TAR))
else
CXXFLAG=-std=c++11
endif

%:%.cpp
	$(GPP) -o $@ -g $< -Wall -Wextra -I ../.. ${CXXFLAG}
	@echo ============ run $@ ================
	@./$@
	@-rm $@
	@-rm -rf $@.dSYM

tar:$(TAR)
	@echo -------test done-------
