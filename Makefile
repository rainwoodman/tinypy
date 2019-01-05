TINYPYC=./tpc

CORELIB_FILES=builtins.py
COMPILER_FILES=boot.py encode.py parse.py py2bc.py tokenize.py

COMPILER_C_FILES=$(COMPILER_FILES:%.py=tinypy/compiler/%.c)
CORELIB_C_FILES=$(CORELIB_FILES:%.py=tinypy/corelib/%.c)

VMLIB_FILES=interp.c dummy-compiler.c corelib.c
TPLIB_FILES=interp.c compiler.c corelib.c

%.c : %.py
	$(TINYPYC) -co $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -g -O0 -I tinypy -c -o $@ $<

all: tpy tpvm

bc: $(BC_FILES)

tinypy/interp.o : tinypy/interp.c tinypy/interp/*.c tinypy/interp.h
tinypy/compiler.o : $(COMPILER_C_FILES) tinypy/compiler.c tinypy/interp.h
tinypy/corelib.o : $(CORELIB_C_FILES) tinypy/corelib.c tinypy/interp.h

# tpvm only takes compiled byte codes (.tpc files)
tpvm : $(VMLIB_FILES:%.c=tinypy/%.o) tinypy/vmmain.o 
	$(CC) -o $@ $^ -lm
	
# tpy takes .py files
tpy : $(TPLIB_FILES:%.c=tinypy/%.o) tinypy/tpmain.o
	$(CC) -o $@ $^ -lm

clean:
	rm -rf tpy tpvm
	rm -rf $(CORELIB_C_FILES)
	rm -rf $(COMPILER_C_FILES)
	rm -rf tinypy/*.o
