ifeq ($(GPP),)
GPP=g++
endif

ifeq ($(NG),)
XFLAG=-DXGTEST
endif

INC=-I ../.. -I ../../googletest/googletest/include  -I /usr/include/qt4  -I /usr/include/qt4/QtCore
LIB=-L ../../googletest/lib -l gtest -pthread -lQtCore

json:
	$(GPP) -o $@ -g test.cpp -std=c++11 -DXPACK_SUPPORT_CHAR_ARRAY -DXPACK_SUPPORT_QT $(INC) $(LIB) $(XFLAG)
	@-valgrind --tool=memcheck --leak-check=full ./$@
	#@- ./$@
	@-rm $@
