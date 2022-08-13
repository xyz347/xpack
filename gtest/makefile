ifeq ($(GPP),)
GPP=g++
endif

MFLAG=-Wall -Wextra

ifneq ($(c11),off)
MFLAG+=-DXGTEST -std=c++11
endif

INC=-I ../.. -I ../../googletest/googletest/include  -I /usr/include/qt4  -I /usr/include/qt4/QtCore
LIB=-L ../../googletest/lib -l gtest -pthread -lQtCore


json:
	$(GPP) -o $@ -g $(MFLAG) test.cpp -DXPACK_SUPPORT_QT $(INC) $(LIB)
	@-valgrind --tool=memcheck --leak-check=full ./$@
	#@- ./$@
	@-rm $@
