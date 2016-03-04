#include "ctlfs.h"

#include <stdio.h>
#include <stdlib.h>

#include <netlink/netlink.h>
#include <netlink/socket.h>

void init_ctlfs(void){

	if(ctlfs_init_dirs() < 0){
		printf("Cannot init ctlfs directories\n");
		exit(1);
	}
	
	if (ctlfs_init_files() < 0){
		printf("Cannot init ctlfs files\n");
		exit(1);
	}
}

void destroy_ctlfs(void){
	
	if (ctlfs_stop_files() < 0){
		printf("cannot destroy files\n");
		exit(1);
	}

	if(ctlfs_stop_dirs() < 0){
		printf("cannot destroy directories\n");
		exit(1);
	}
}


int main(){

    printf("Initializing clondike kernel simulator\n");


    init_ctlfs();
    //destroy_ctlfs();

    struct nl_sock* sk = nl_socket_alloc();





    nl_socket_free(sk);

}

