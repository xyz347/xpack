SRC=$(wildcard *.cpp)
TAR=$(SRC:.cpp=.exe)

%.exe:%.cpp
	@cl /Fe:${CURDIR}/$@ $< /EHsc /nologo /wd4819 -I ../.. >nul
	@echo ============ run $@ ================
	@$@

tar:$(TAR)
	@echo -------test done-------
	@-del *.obj
	@-del *.exe
