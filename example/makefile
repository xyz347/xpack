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

mysql:mysql.cpp
	$(GPP) -o $@ -g $< -lmysqlclient -I ../..

sqlite:sqlite.cpp
	$(GPP) -o $@ -g $< -lsqlite -I ../..

yaml:yaml.cpp
	$(GPP) -o $@ -g $< -Wall -Wextra -I ../.. -I ../../yaml-cpp/include  ../../yaml-cpp/build/libyaml-cpp.a -std=c++11
	@echo ============ run $@ ================
	@./$@
	@-rm $@
	@-rm -rf $@.dSYM