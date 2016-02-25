CC=gcc
CFLAGS=-std=c99 -Wall
LIBS=-lm
TARGET=build/tinypy
OBJS=	tinypy/list.o 		\
	tinypy/gc.o		\
	tinypy/vm.o		\
	tinypy/dict.o		\
	tinypy/string.o 	\
	tinypy/misc.o		\
	tinypy/builtins.o 	\
	tinypy/ops.o		\
	tinypy/tp.o		\
	tinypy/mymain.o

TEST=examples/hw.py

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LIBS) $(OBJS) -o $(TARGET)

.PHONY:clean distclean test

test:
	./$(TARGET) $(TEST)

clean:
	rm -f $(OBJS)

distclean: clean
	rm -f $(TARGET)
