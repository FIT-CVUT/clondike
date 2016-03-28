CC=gcc
CXX=g++
OBJECTS= clondike_kernel_simulator.o ctlfs.o ctlfs_file_helper.o message_helper.o message_task_fork.o response_handlers.o message_task_exit.o message_npm_check.o message_node_connected.o message_node_disconnected.o message_immigration_request.o message_immigration_confirmed.o message_emigration_failed.o pen_watcher.o kkc.o kkc_messages.o process_manager.o fifo_reader.o netlink_message.o pid_manager.o worker.o kkc_socket_manager.o message_generic_user_message.o
CCFLAGS = -g -I/usr/include/libnl3 -lpthread 
CXXFLAGS = -Wall -g -I/usr/include/libnl3 -Wno-long-long 
LDFLAGS = -lnl-genl-3 -lnl-3 -lpthread
EXEC=clondike_kernel_simulator

all: clondike_kernel_simulator

debug:	CXXFLAGS += -DDEBUG
debug:	CCFLAGS += -DDEBUG
debug:	clondike_kernel_simulator

clondike_kernel_simulator: $(OBJECTS)
	$(CXX) -o $(EXEC) $(LDFLAGS) $(OBJECTS)

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $<

%.o : %.c
	$(CC) -c $(CCFLAGS) $<

clean:
	rm -rf *.o $(EXEC)

