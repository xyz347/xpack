ifeq ($(GPP),)
GPP=g++
endif

MFLAG=-Wall -Wextra

INC=-I ../..
LIB=-pthread

# c11 default on
ifneq ($(c11),off)
  MFLAG+=-std=c++11
  # gtest default off
  ifeq ($(gtest),on)
    INC+=-I ../../googletest/googletest/include
    LIB+=-L ../../googletest/lib -lgtest
    MFLAG+=-DXGTEST
  endif
endif

# QT default off
ifeq ($(qt),on)
    INC+=-I /usr/include/qt4  -I /usr/include/qt4/QtCore
    LIB+=-lQtCore
    MFLAG+=-DXPACK_SUPPORT_QT
endif

ifneq ($(xout),)
MFLAG+=-DXPACK_OUT_TEST
endif

xtest:
	$(GPP) -o $@ -g $(MFLAG) test.cpp  $(INC) $(LIB)
	@-valgrind --tool=memcheck --leak-check=full ./$@
	@-rm $@

test:
	$(GPP) -o $@ -g $(MFLAG) test.cpp  $(INC) $(LIB)
	@-./$@
	@-rm $@

bson:bson_test.cpp
	$(GPP) -o $@ -g $< -std=c++11 $(INC) $(LIB) $(MFLAG) -lbson-1.0
	@-valgrind --tool=memcheck --leak-check=full	./$@
	@-rm $@