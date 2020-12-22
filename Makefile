TINYPYC=./tpc

RUNTIME_FILES=types.py testing.py
COMPILER_FILES=boot.py encode.py parse.py py2bc.py tokenize.py opcodes.py

COMPILER_C_FILES=$(COMPILER_FILES:%.py=tinypy/compiler/%.c)
RUNTIME_C_FILES=$(RUNTIME_FILES:%.py=tinypy/runtime/%.c)

VMLIB_FILES=tp.c dummy-compiler.c runtime.c
TPLIB_FILES=tp.c compiler.c runtime.c

MODULES=math random re
MODULES_A_FILES=$(MODULES:%=modules/%.a)
MODULES_C_FILES=$(MODULES:%=modules/%/init.c)

TESTS_PY_FILES=$(wildcard tests/*.py)

%.c : %.py
	$(TINYPYC) -co $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -g -O0 -I . -c -o $@ $<

all: tpy tpvm

# alias
lib: libtpy.so

modules/modules.c: $(MAKEFILE)
	echo "#include <tinypy/tp.h>" > $@
	for name in $(MODULES); do echo "void $${name}_init(TP);" >> $@; done
	echo "void _tp_import_modules(TP) {" >> $@
	for name in $(MODULES); do echo "$${name}_init(tp);" >> $@; done
	echo "}" >> $@

modules/modules.a: modules/modules.o \
			$(MODULES_C_FILES:%.c=%.o)
	$(AR) rcu $@ $^

tinypy/tp_opcodes.h: tinypy/compiler/opcodes.py
	$(TINYPYC) -x -o $@

tinypy/tp.o : tinypy/tp.c tinypy/tp*.c tinypy/tp*.h tinypy/tp_opcodes.h tinypy/printf/*.c tinypy/printf/*.h
tinypy/compiler.o : $(COMPILER_C_FILES) tinypy/compiler.c tinypy/*.h
tinypy/runtime.o : $(RUNTIME_C_FILES) tinypy/runtime.c tinypy/*.h
#
# tpvm only takes compiled byte codes (.tpc files)
tpvm : $(VMLIB_FILES:%.c=tinypy/%.o) tinypy/vmmain.c modules/modules.a
	$(CC) -o $@ $^ -lm
#
tpvm-dbg : $(VMLIB_FILES:%.c=tinypy/%.o) tinypy/vmmain.c modules/modules.a
	$(CC) -D TPVM_DEBUG -o $@ $^ -lm
	
# tpy takes .py files
tpy : $(TPLIB_FILES:%.c=tinypy/%.o) tinypy/tpmain.c modules/modules.a
	$(CC) -o $@ $^ -lm

libtpy.so : CFLAGS += -fPIC
libtpy.so : $(TPLIB_FILES:%.c=tinypy/%.o)
	$(CC) -shared $(CFLAGS) -lm -Wl,-soname,$(@) -o $@ $^
	#ln -s $@.1 $@

test: $(TESTS_PY_FILES) tpy tpvm tpvm-dbg run-tests.sh
	bash run-tests.sh $(TESTS_PY_FILES)

xtest: $(TESTS_PY_FILES) tpy tpvm tpvm-dbg run-tests.sh
	bash run-tests.sh --xfail $(TESTS_PY_FILES)

test-dbg: $(TESTS_PY_FILES) tpy tpvm tpvm-dbg run-tests.sh
	bash run-tests.sh --backend=tpvm --debug $(TESTS_PY_FILES)

test-tpy: $(TESTS_PY_FILES) tpy tpvm tpvm-dbg run-tests.sh
	bash run-tests.sh --backend=tpy $(TESTS_PY_FILES)

clean:
	rm -rf tpy tpvm tpvm-dbg libtpy.so
	rm -rf $(RUNTIME_C_FILES)
	rm -rf $(COMPILER_C_FILES)
	rm -rf tinypy/*.o
	rm -rf tinypy/tp_opcodes.h
	rm -rf modules/*/*.o
	rm -rf modules/*.a
	rm -rf modules/modules.c
