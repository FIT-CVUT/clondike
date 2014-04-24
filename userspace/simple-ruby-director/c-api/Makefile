OBJECTS = director-cli.o
CFLAGS += -g -fPIC
AR		= ar

all: director-cli.a test-bin

director-cli: $(OBJECTS)
	$(CC) $(OBJECTS) -L$(OPENSSL_FUNC) -I../../../sources/libnl-3/include -L../../../sources/libnl-3/lib/.libs -L/usr/lib -lnl-3 -lssl -lopensslfunc -o ossld

director-cli.a: $(OBJECTS)
	$(AR) rcs libdirector-cli.a $(OBJECTS)

test-bin: test.o director-cli.a
	$(CC) test.o -L$(OPENSSL_FUNC) -I../../../sources/libnl-3/include -L. -L/usr/lib -ldirector-cli  -o test-bin

clean:
	rm -f *.a *.so *.o
