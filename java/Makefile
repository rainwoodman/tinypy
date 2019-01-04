CFLAGS=-fPIC -I$(JAVA_HOME)/../include/ -I$(JAVA_HOME)/../include/linux -pipe -g -Wall

run: TinyPy.class libtinypy.so
	java -Djava.library.path=.  -cp . TinyPy

%.class: %.java
	javac $<

libtinypy.so: libtinypy.c
	$(CC) -o $@ -shared -Wl,-soname,$@ $^ $(CFLAGS)

clean:
	rm -f *.log TinyPy.class libtinypy.so
