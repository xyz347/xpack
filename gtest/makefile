ifeq ($(GPP),)
GPP=g++
endif


INC=-I ../ -I ../../googletest/googletest/include  -I /usr/include/qt4  -I /usr/include/qt4/QtCore
LIB=-L ../../googletest/lib -l gtest -pthread -lQtCore

json:
	$(GPP) -o $@ -g json.cpp -std=c++11 -DXPACK_SUPPORT_CHAR_ARRAY -DXPACK_SUPPORT_QT $(INC) $(LIB)
	@-valgrind --tool=memcheck --leak-check=full ./$@
	@-rm $@
