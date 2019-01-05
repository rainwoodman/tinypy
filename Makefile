TINYPYC=./tinypyc

PYFILES=encode.py parse.py py2bc.py

tinypy/corelib/%.c : tinypy/compiler/%.py
	@mkdir -p `dirname $@`
	$(TINYPYC) -co $@ $^

bytecodes: $(PYFILES:%.py=tinypy/corelib/%.c)


