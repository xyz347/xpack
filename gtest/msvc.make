SRC=$(wildcard *.cpp)
TAR=$(SRC:.cpp=.exe)

ifneq ($(xout),)
MFLAG+=-DXPACK_OUT_TEST
endif

%.exe:%.cpp
	cl /Fe:${CURDIR}/$@ $< /EHsc /nologo /wd4819 -I ../.. $(MFLAG)
	@echo ============ run $@ ================
	@$@

tar:$(TAR)
	@echo -------test done-------
	@-del *.obj
	@-del *.exe
