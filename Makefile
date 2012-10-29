CFLAGS=-Wall -I./src
CC=gcc
VERSION=0.0.1

# Set the debug macro for compilation if requested
ifeq ($(DEBUG), true)
	CFLAGS += -DDEBUG -g
endif

liboptbot.so: build/liboptbot.o
	$(CC) -shared -Wl,-soname,liboptbot.so.$(VERSION) \
	  -o lib/liboptbot.so.$(VERSION) build/liboptbot.o
	ln lib/liboptbot.so.$(VERSION) lib/liboptbot.so
example: liboptbot.so
	gcc -L/home/jack/dht/lib/ examples/basic.c -loptbot -o bin/basic_example -I./src
build/liboptbot.o: src/liboptbot.c
	$(CC) $(CFLAGS) -fPIC -c src/liboptbot.c -o build/liboptbot.o
bin/test: build/liboptbot.o
	$(CC) $(CFLAGS) -Wall -I./src build/liboptbot.o test/main.c -lcheck -o bin/test
test: bin/test
	./bin/test
.PHONY: test
clean:
	rm -f build/*.o bin/* lib/*
	rm -rf doc/*
.PHONY: clean
doc:
	doxygen doxygen.conf
.PHONY: doc
doc_server: doc
	cd doc/html; python -m SimpleHTTPServer
.PHONY: doc_server
