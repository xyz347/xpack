SRC=$(wildcard *.cpp)
TAR=$(SRC:.cpp=.exe)

%.exe:%.cpp
	cl /Fe:${CURDIR}/$@ $< /EHsc -I ../.. -DXPACK_SUPPORT_CHAR_ARRAY
	@echo ============ run $@ ================
	@$@

tar:$(TAR)
	@echo -------test done-------
	@-del *.obj
	@-del *.exe
