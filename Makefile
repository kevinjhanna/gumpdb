CC=gcc
CFLAGS=-I.
DEPS = src/gump_db.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

test: src/gump_db.o test/test.o
	$(CC) -o tests.out src/gump_db.o test/test.o -I.

.PHONY: clean

clean:
	rm -f src/*.o && rm -f *.out

