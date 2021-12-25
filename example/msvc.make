SRC=$(wildcard *.cpp)
TAR=$(SRC:.cpp=.exe)

%.exe:%.cpp
	@cl /Fe:${CURDIR}/$@ /nologo /wd4819 $< /EHsc -I ../.. 
	@echo ============ run $@ ================
	@$@

tar:$(TAR)
	@echo -------test done-------
	@-del *.obj
	@-del *.exe
