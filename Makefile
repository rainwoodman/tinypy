TINYPYC=./tpc

RUNTIME_FILES=types.py testing.py
COMPILER_FILES=boot.py encode.py parse.py py2bc.py tokenize.py opcodes.py

TPY_DEP_FILES=tinypy/tp.c \
            tinypy/tp*.c tinypy/tp*.h \
            tinypy/tp_opcodes.h \
            tinypy/printf/*.c tinypy/printf/*.h

COMPILER_DEP_FILES = tinypy/compiler.c
COMPILER_DEP_FILES+= tinypy/*.h \
                   $(COMPILER_FILES:%.py=tinypy/compiler/%.c)

RUNTIME_DEP_FILES = tinypy/runtime.c
RUNTIME_DEP_FILES+= tinypy/*.h \
                  $(RUNTIME_FILES:%.py=tinypy/runtime/%.c)

VMLIB_FILES=tp.c dummy-compiler.c runtime.c
TPLIB_FILES=tp.c compiler.c runtime.c

MODULES=math random re
MODULES_A_FILES=$(MODULES:%=modules/%.a)
MODULES_C_FILES=$(MODULES:%=modules/%/init.c)

TESTS_PY_FILES=$(wildcard tests/*.py)

%.c : %.py
	$(TINYPYC) -co $@ $^

.objs/%.o : %.c
	@mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -g -O0 -I . -c -o $@ $<

.dynobjs/%.o : %.c
	@mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -fPIC -g -O0 -I . -c -o $@ $<

all: tpy tpvm

shared: tpy-shared

# alias
lib: libtpy.so

modules/modules.c: $(MAKEFILE)
	echo "#include <tinypy/tp.h>" > $@
	for name in $(MODULES); do echo "void $${name}_init(TP);" >> $@; done
	echo "void _tp_import_modules(TP) {" >> $@
	for name in $(MODULES); do echo "$${name}_init(tp);" >> $@; done
	echo "}" >> $@

modules/modules.a: .objs/modules/modules.o \
			$(MODULES_C_FILES:%.c=.objs/%.o)
	$(AR) rcu $@ $^

modules/modules.dyn.a: .dynobjs/modules/modules.o \
			$(MODULES_C_FILES:%.c=.dynobjs/%.o)
	$(AR) rcu $@ $^

tinypy/tp_opcodes.h: tinypy/compiler/opcodes.py
	$(TINYPYC) -x -o $@

.objs/tinypy/tp.o    : $(TPY_DEP_FILES)
.dynobjs/tinypy/tp.o : $(TPY_DEP_FILES)
.objs/tinypy/compiler.o    : $(COMPILER_DEP_FILES)
.dynobjs/tinypy/compiler.o : $(COMPILER_DEP_FILES)
.objs/tinypy/runtime.o    : $(RUNTIME_DEP_FILES)
.dynobjs/tinypy/runtime.o : $(RUNTIME_DEP_FILES)

# tpvm only takes compiled byte codes (.tpc files)
tpvm : $(VMLIB_FILES:%.c=.objs/tinypy/%.o) tinypy/vmmain.c modules/modules.a
	$(CC) -o $@ $^ -lm

tpvm-dbg : $(VMLIB_FILES:%.c=.objs/tinypy/%.o) tinypy/vmmain.c modules/modules.a
	$(CC) -D TPVM_DEBUG -o $@ $^ -lm

# tpy takes .py files
tpy : $(TPLIB_FILES:%.c=.objs/tinypy/%.o) tinypy/tpmain.c modules/modules.a
	$(CC) -o $@ $^ -lm

libtpy.so : $(TPLIB_FILES:%.c=.dynobjs/tinypy/%.o) \
            .dynobjs/modules/modules.o \
            $(MODULES_C_FILES:%.c=.dynobjs/%.o)
	$(CC) -shared $(CFLAGS) -lm -Wl,-soname,$(@) -o $@ $^
	#ln -s $@.1 $@

tpy-shared : tinypy/tpmain.c libtpy.so
	$(CC) -o $@ $^ -lm -Wl,-rpath,'$$ORIGIN'

test: $(TESTS_PY_FILES) tpvm run-tests.sh
	bash run-tests.sh --backend=tpvm $(TESTS_PY_FILES)

xtest: $(TESTS_PY_FILES) tpvm tpvm-dbg run-tests.sh
	bash run-tests.sh --backend=tpvm --xfail $(TESTS_PY_FILES)

test-dbg: $(TESTS_PY_FILES) tpvm-dbg run-tests.sh
	bash run-tests.sh --backend=tpvm-dbg $(TESTS_PY_FILES)

test-tpy: $(TESTS_PY_FILES) tpy run-tests.sh
	bash run-tests.sh --backend=tpy $(TESTS_PY_FILES)

test-shared: $(TESTS_PY_FILES) tpy-shared run-tests.sh
	bash run-tests.sh --backend=tpy-shared $(TESTS_PY_FILES)

clean:
	rm -rf tpy tpvm tpvm-dbg libtpy.so
	rm -rf $(COMPILER_FILES:%.py=tinypy/compiler/%.c)
	rm -rf $(RUNTIME_FILES:%.py=tinypy/compiler/%.c)
	rm -rf .objs/
	rm -rf .dynobjs/
	rm -rf tinypy/tp_opcodes.h
	rm -rf modules/*.a
	rm -rf modules/modules.c
