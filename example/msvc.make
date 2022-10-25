MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-builtin-variables
SRC=$(wildcard *.cpp)
TAR=$(SRC:.cpp=)

%:%.cpp
	cl /Fe:${CURDIR}/$@.exe /nologo /wd4819 $< /EHsc -I ../..
	@echo ============ run $@.exe ================
	@$@.exe
	@del $@.exe $@.obj

tar:$(TAR)
	@echo -------test done-------

clean:
	@-del *.obj *.exe
