CC=gcc
OBJECTS = msg-common.o director-api.o npm-check.o node-connected.o node-disconnected.o immigration-request.o immigration-confirmed.o task-exitted.o task-for$
CFLAGS += -g -fPIC -I../../sources/libnl-3/include -lnl-3 -lssl
AR              = ar

all: director-api.a

director-api: $(OBJECTS)
        $(CC) $(OBJECTS)

director-api.a: $(OBJECTS)
        $(AR) rcs libdirector-api.a $(OBJECTS)


clean:
        rm -f *.a *.so *.o

