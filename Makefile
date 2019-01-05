TINYPYC=./tpc

CORELIB_FILES=boot.py encode.py parse.py py2bc.py tokenize.py

VMLIB_FILES=vm.c dummy-compiler.c
TPLIB_FILES=vm.c compiler.c

tinypy/corelib/%.c : tinypy/compiler/%.py
	@mkdir -p `dirname $@`
	$(TINYPYC) -co $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -I tinypy -c -o $@ $<

bc: $(CORELIB_FILES:%.py=tinypy/corelib/%.c)

vm : $(VMLIB_FILES:%.c=tinypy/vm/%.o) tinypy/vmmain.o 
	$(CC) -o $@ $^ -lm
	
tinypy/vm/vm.o : tinypy/vm/core.c tinypy/vm/core/*.c tinypy/vm/vm.c tinypy/vm/*.h
tinypy/vm/compiler.o : $(CORELIB_FILES:%.py=tinypy/corelib/%.c) tinypy/vm/compiler.c tinypy/vm/*.h

tp : $(TPLIB_FILES:%.c=tinypy/vm/%.o) tinypy/tpmain.o
	$(CC) -o $@ $^ -lm

clean:
	rm -rf tp vm
	rm -rf tinypy/corelib/*.c
	rm -rf tinypy/*.o
	rm -rf tinypy/vm/*.o
