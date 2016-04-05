CC=gcc
OBJECTS = msg-common.o director-api.o npm-check.o node-connected.o node-disconnected.o immigration-request.o immigration-confirmed.o task-exitted.o task-forked.o generic_user_message.o migrated-home.o emigration-failed.o ack.o
CFLAGS += -g -fPIC -I /usr/include/libnl3 -lnl-3 -lssl
AR = ar

all: director-api.a

director-api: $(OBJECTS)
	$(CC) $(OBJECTS)

director-api.a: $(OBJECTS)
	$(AR) rcs libdirector-api.a $(OBJECTS)


clean:
	rm -f *.a *.so *.o

