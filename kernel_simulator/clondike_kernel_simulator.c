#include "clondike_kernel_simulator.h"
#include "ctlfs.h"
#include "message_helper.h"
#include "msgs.h"
#include "response_handlers.h"
#include "message_task_fork.h"
#include "message_task_exit.h"
#include "message_node_connected.h"
#include "kkc.h"
#include "pen_watcher.h"
#include "netlink_message.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/genl/mngt.h>
#include <netlink/genl/genl.h>


static int run_process_cleaner = 0;



void sig_handler(int signo)
{
    if (signo == SIGINT){
        printf("received SIGINT\n");
        kkc_close_connections();
        netlink_close();
        destroy_ctlfs();
        exit(0);
    }
}

void should_run_process_cleaner(){
    run_process_cleaner = 1;    
}

int main(){

	int ret = 0;
    printf("Initializing clondike kernel simulator\n");

    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");

    ret = init_ctlfs();
    if (ret < 0){
        printf("ERROR: cannot initialize ctlfs\n");
        return ret;
    }

    ret = init_netlink();
    if (ret < 0){
        printf("ERROR: cannot init netlink\n");
        return ret;
    }

    init_process_reader();

    init_worker();

    if (start_ccn() == -1){
        printf("cannot start ccn manager, terminating!\n");
        return 1;
    }

    int pen;

    int cycle = 0;

    while(1){

        try_netlink_receive();

        if(check_pen_watcher()){
            ccn_connect();
        }

        try_receive_ccn();

        //kkc_send_emig_request(get_socket(0), 1234, 999, "/usr/lib/src");
        
        try_read_processes();
        
        emig_send_messages();
        imig_send_messages();

        if (cycle%1000 == 0){
            netlink_send_task_fork(get_next_pid(), get_next_pid());
        }
        ++cycle;
        //printf("cycle: %d\n", cycle);

        if(run_process_cleaner){
            process_cleaner();
            run_process_cleaner = 0;
        }

        usleep(500);
        
        //do maintain every 1000 cycles
        if (cycle%100 == 0){
            kkc_erase_disconnected_sockets();
        }

    }


    destroy_ctlfs();
   
    close_fifo();
    destroy_fifo();

    close_connections();

    return 0;
}

