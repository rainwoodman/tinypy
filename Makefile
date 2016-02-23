CC=gcc
CFLAGS=-std=c89 -Wall
LIBS=-lm
TARGET=build/tinypy
OBJS=tinypy/mymain.o
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
