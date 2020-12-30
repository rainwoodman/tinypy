TINYPYC=./tpc

GENERATED_SOURCE_FILES=

RUNTIME_FILES=tinypy/runtime/types.py \
              tinypy/runtime/testing.py
GENERATED_SOURCE_FILES+=$(RUNTIME_FILES:%.py=%.c)

COMPILER_FILES=tinypy/compiler/boot.py \
               tinypy/compiler/encode.py \
               tinypy/compiler/encode.py \
               tinypy/compiler/parse.py \
               tinypy/compiler/py2bc.py \
               tinypy/compiler/tokenize.py \
               tinypy/compiler/opcodes.py
GENERATED_SOURCE_FILES+=$(COMPILER_FILES:%.py=%.c)


TPY_DEP_FILES=tinypy/tp.c \
            tinypy/tp*.c tinypy/tp*.h \
            tinypy/tp_opcodes.h \
            tinypy/printf/*.c tinypy/printf/*.h

COMPILER_DEP_FILES = tinypy/compiler.c
COMPILER_DEP_FILES+= tinypy/*.h $(COMPILER_FILES:%.py=%.c)

RUNTIME_DEP_FILES = tinypy/runtime.c
RUNTIME_DEP_FILES+= tinypy/*.h $(RUNTIME_FILES:%.py=%.c)

VMLIB_FILES=tp.c dummy-compiler.c runtime.c
TPLIB_FILES=tp.c compiler.c runtime.c

MODULES=math random re
MODULES_A_FILES=$(MODULES:%=modules/%.a)
MODULES_C_FILES=$(MODULES:%=modules/%/init.c)

TESTS_PY_FILES=$(wildcard tests/*.py)

# rule to compile python scripts to bytecodes as c source code.
%.c : %.py
	@mkdir -p $(dir $@)
	$(TINYPYC) -co $@ $^

# rule to make objects for static linkage
.objs/%.o : %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -g -O0 -I . -c -o $@ $<

.dbgobjs/%.o : %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -DTPVM_DEBUG -g -O0 -I . -c -o $@ $<

# rule to make objects for dynamic linkage
.dynobjs/%.o : %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -fPIC -g -O0 -I . -c -o $@ $<


all: tpy tpvm

# do not link libtpy.so by default.
shared: tpy-shared

# alias
lib: libtpy.so

modules/modules.c: $(MAKEFILE)
	echo "#include <tinypy/tp.h>" > $@
	for name in $(MODULES); do echo "void $${name}_init(TP);" >> $@; done
	echo "void _tp_import_modules(TP) {" >> $@
	for name in $(MODULES); do echo "$${name}_init(tp);" >> $@; done
	echo "}" >> $@
GENERATED_SOURCE_FILES+=modules/modules.c

modules/modules.a: .objs/modules/modules.o \
			$(MODULES_C_FILES:%.c=.objs/%.o)
	$(AR) rcu $@ $^

tinypy/tp_opcodes.h: tinypy/compiler/opcodes.py
	@mkdir -p $(dir $@)
	$(TINYPYC) -x -o $@
GENERATED_SOURCE_FILES+=tinypy/tp_opcodes.h

# extra dependencies
.objs/tinypy/tp.o    : $(TPY_DEP_FILES)
.dbgobjs/tinypy/tp.o    : $(TPY_DEP_FILES)
.dynobjs/tinypy/tp.o : $(TPY_DEP_FILES)
.objs/tinypy/compiler.o    : $(COMPILER_DEP_FILES)
.dbgobjs/tinypy/compiler.o    : $(COMPILER_DEP_FILES)
.dynobjs/tinypy/compiler.o : $(COMPILER_DEP_FILES)
.objs/tinypy/runtime.o    : $(RUNTIME_DEP_FILES)
.dbgobjs/tinypy/runtime.o    : $(RUNTIME_DEP_FILES)
.dynobjs/tinypy/runtime.o : $(RUNTIME_DEP_FILES)

# tpvm only takes compiled byte codes (.tpc files)
tpvm : $(VMLIB_FILES:%.c=.objs/tinypy/%.o) .objs/tinypy/vmmain.o modules/modules.a
	$(CC) -g -O0 -o $@ $^ -lm
#
# tpvm only takes compiled byte codes (.tpc files)
tpvm-dbg : $(VMLIB_FILES:%.c=.dbgobjs/tinypy/%.o) .dbgobjs/tinypy/vmmain.o modules/modules.a
	$(CC) -g -O0 -o $@ $^ -lm

# tpy takes .py files
tpy : $(TPLIB_FILES:%.c=.objs/tinypy/%.o) tinypy/tpmain.c modules/modules.a
	$(CC) -o $@ $^ -lm

# rule for making a shared object. This is not universal and shall be adapted.
libtpy.so : $(TPLIB_FILES:%.c=.dynobjs/tinypy/%.o) \
            .dynobjs/modules/modules.o \
            $(MODULES_C_FILES:%.c=.dynobjs/%.o)
	$(CC) -shared $(CFLAGS) -lm -Wl,-soname,$(@) -o $@ $^
	#ln -s $@.1 $@

# rule for making a shared executable. This is not universal and shall be adapted.
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
	rm -rf $(GENERATED_SOURCE_FILES)
	rm -rf .objs/
	rm -rf .dbgobjs/
	rm -rf .dynobjs/
	rm -rf modules/*.a
