TINYPYC=./tpc

RUNTIME_FILES=types.py
COMPILER_FILES=boot.py encode.py parse.py py2bc.py tokenize.py run.py

COMPILER_C_FILES=$(COMPILER_FILES:%.py=tinypy/compiler/%.c)
RUNTIME_C_FILES=$(RUNTIME_FILES:%.py=tinypy/runtime/%.c)

VMLIB_FILES=tp.c dummy-compiler.c runtime.c
TPLIB_FILES=tp.c compiler.c runtime.c

MODULES=math random re
MODULES_A_FILES=$(MODULES:%=modules/%.a)
MODULES_C_FILES=$(MODULES:%=modules/%/init.c)

%.c : %.py
	$(TINYPYC) -co $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -g -O0 -I . -c -o $@ $<

all: tpy tpvm

modules/modules.c: $(MAKEFILE)
	echo "#include <tinypy/tp.h>" > $@
	for name in $(MODULES); do echo "void $${name}_init(TP);" >> $@; done
	echo "void _tp_import_modules(TP) {" >> $@
	for name in $(MODULES); do echo "$${name}_init(tp);" >> $@; done
	echo "}" >> $@

modules/modules.a: modules/modules.o \
			$(MODULES_C_FILES:%.c=%.o)
	$(AR) rcu $@ $^

bc: $(BC_FILES)

tinypy/tp.o : tinypy/tp.c tinypy/tp*.c tinypy/tp*.h
tinypy/compiler.o : $(COMPILER_C_FILES) tinypy/compiler.c tinypy/*.h
tinypy/runtime.o : $(RUNTIME_C_FILES) tinypy/runtime.c tinypy/*.h
#
# tpvm only takes compiled byte codes (.tpc files)
tpvm : $(VMLIB_FILES:%.c=tinypy/%.o) tinypy/vmmain.o modules/modules.a
	$(CC) -o $@ $^ -lm
	
# tpy takes .py files
tpy : $(TPLIB_FILES:%.c=tinypy/%.o) tinypy/tpmain.o modules/modules.a
	$(CC) -o $@ $^ -lm

clean:
	rm -rf tpy tpvm
	rm -rf $(RUNTIME_C_FILES)
	rm -rf $(COMPILER_C_FILES)
	rm -rf tinypy/*.o
	rm -rf modules/*/*.o
	rm -rf modules/*.a
	rm -rf modules/modules.c
