CC=gcc
OBJECTS= clondike_kernel_simulator.o ctlfs.o ctlfs_file_helper.o
CFLAGS += -g -I/usr/include/libnl3 
LDFLAGS = -lnl-3 -lnl-genl-3 
EXEC=clondike_kernel_simulator

all: clondike_kernel_simulator

clondike_kernel_simulator: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(EXEC)

clean:
	rm -rf *.o $(EXEC)

