TINYPYC=./tpc

CORELIB_FILES=boot.py encode.py parse.py py2bc.py tokenize.py

VMLIB_FILES=interp.c dummy-compiler.c
TPLIB_FILES=interp.c compiler.c

tinypy/corelib/%.c : tinypy/compiler/%.py
	@mkdir -p `dirname $@`
	$(TINYPYC) -co $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -I tinypy -c -o $@ $<

all: tpy tpvm

bc: $(CORELIB_FILES:%.py=tinypy/corelib/%.c)

tinypy/interp.o : tinypy/interp.c tinypy/interp/*.c tinypy/interp.h
tinypy/compiler.o : $(CORELIB_FILES:%.py=tinypy/corelib/%.c) tinypy/compiler.c tinypy/interp.h

# tpvm only takes compiled byte codes (.tpc files)
tpvm : $(VMLIB_FILES:%.c=tinypy/%.o) tinypy/vmmain.o 
	$(CC) -o $@ $^ -lm
	
# tpy takes .py files
tpy : $(TPLIB_FILES:%.c=tinypy/%.o) tinypy/tpmain.o
	$(CC) -o $@ $^ -lm

clean:
	rm -rf tpy tpvm
	rm -rf tinypy/corelib/*.c
	rm -rf tinypy/*.o
