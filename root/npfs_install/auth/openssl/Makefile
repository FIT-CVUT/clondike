OBJECTS = openssl-auth-module.o fifo_buffer.o openssl-crypt.o
SERVER =../../include
OPENSSL_FUNC =../../../openssl-func
ACCESS_CONFIG =../../access-config
CFLAGS += -g -I$(OPENSSL_FUNC) -I$(SERVER) -I$(ACCESS_CONFIG)

all: opensslauthmodule

opensslauthmodule: $(OBJECTS)
	$(AR) rcs libopensslauthmodule.a $(OBJECTS)

clean:
	rm -f *.a *.o