CC=gcc
CXX=g++
OBJECTS= clondike_kernel_simulator.o ctlfs.o ctlfs_file_helper.o message_helper.o message_task_fork.o response_handlers.o message_task_exit.o message_npm_check.o message_node_connected.o message_immigration_request.o message_immigration_confirmed.o pen_watcher.o kkc.o
CCFLAGS = -g -I/usr/include/libnl3 
CXXFLAGS = -Wall -pedantic -g -ansi
LDFLAGS = -lnl-genl-3 -lnl-3
EXEC=clondike_kernel_simulator

all: clondike_kernel_simulator

clondike_kernel_simulator: $(OBJECTS)
	$(CXX) -o $(EXEC) $(LDFLAGS) $(OBJECTS) 

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $<

%.o : %.c
	$(CC) -c $(CCFLAGS) $<


clean:
	rm -rf *.o $(EXEC)

