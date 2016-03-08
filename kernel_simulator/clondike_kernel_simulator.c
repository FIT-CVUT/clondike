#include "ctlfs.h"
#include "message_helper.h"
#include "msgs.h"

#include <stdio.h>
#include <stdlib.h>

#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>

#define LOCAL_NETLINK_PORT 111111111

int director_pid = 0;

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

void set_director_pid(struct nl_msg *msg){
    if (director_pid == 0){
        director_pid = nlmsg_hdr(msg)->nlmsg_pid;
    }
}

int check_registered_director_pid(struct nl_msg * msg){
    struct nlattr *nla;
    nla = nlmsg_find_attr(nlmsg_hdr(msg), sizeof(struct genlmsghdr), DIRECTOR_A_PID);
    if(nla == NULL){
        return -1;
    }

    if (nla_get_u32(nla) != director_pid){
        printf("Registered PID mismatch!\n");
        return -255;
    }
    return 0;
}

int callback_message(struct nl_msg * msg, void * arg) {
    printf("received message:\n");
    nl_msg_dump(msg, stdout);
    printf("received message end:\n");

    struct genlmsghdr* genl_hdr;
    int cmd;
    genl_hdr = (struct genlmsghdr *)nlmsg_data(nlmsg_hdr(msg));
    cmd = genl_hdr->cmd;

    if (cmd == DIRECTOR_REGISTER_PID){
        check_registered_director_pid(msg);
    }


    set_director_pid(msg);
    printf("director peer port: %d\n", director_pid);

    return 0;
}

int main(){

	int ret_val = 0;
    struct nl_msg * msg;
    struct nlmsghdr *hdr;
    struct genlmsghdr genl_hdr;

    printf("Initializing clondike kernel simulator\n");


    init_ctlfs();
    //destroy_ctlfs();

    struct nl_sock* sk = nl_socket_alloc();
	nl_socket_set_buffer_size(sk, 15000000, 15000000);

	nl_socket_disable_seq_check(sk);

	printf("peer port: %d\n", nl_socket_get_peer_port(sk));
	printf("local port: %d\n", nl_socket_get_local_port(sk));

	nl_socket_set_local_port(sk, LOCAL_NETLINK_PORT);

printf("local port: %d\n", nl_socket_get_local_port(sk));

	if ( (ret_val=nl_connect(sk, NETLINK_GENERIC)) ){
		printf("cannot connect socket\n");
	}

    nl_socket_modify_cb(sk, NL_CB_MSG_IN, NL_CB_CUSTOM, callback_message, NULL);

	int res = nl_recvmsgs_default(sk);
    printf("after receive %d\n", res);
	printf("peer port: %d\n", nl_socket_get_peer_port(sk));

    printf("set peer port\n");
    nl_socket_set_peer_port(sk, director_pid);



//register family
    
    //this will alocate new message, has to be freed after using
    prepare_message(CTRL_CMD_NEWFAMILY, &msg);
    
    nla_put_string(msg, CTRL_ATTR_FAMILY_ID, "DIRECTORCHNL");

    send_message(sk, msg);

    nlmsg_free(msg);

    

//registering pid
//
//
    nl_recvmsgs_default(sk);

    prepare_message(DIRECTOR_ACK, &msg);
    send_message(sk, msg);







    nl_socket_free(sk);

}

