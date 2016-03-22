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

#define LOCAL_NETLINK_PORT 111111111

static int director_pid = 0;
static struct nl_sock* sk;
static int run_process_cleaner = 0;

struct genl_cmd my_genl_cmds[] = {
    {
        .c_id = DIRECTOR_ACK,
        .c_name = "DIRECTOR_ACK",
        .c_msg_parser = ack_handler,
    },
};

struct genl_ops my_genl_ops = {
    .o_name = "DIRECTORCHNL",
    .o_cmds = my_genl_cmds,
    .o_ncmds = 1,
};

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

static void set_director_pid(struct nl_msg *msg){
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

void get_family_id(struct nl_msg *msg){
    int nl_family_id = 0;
    printf("get family id\n");
    struct nlattr *nla;
    nla = nlmsg_find_attr(nlmsg_hdr(msg), sizeof(struct genlmsghdr), 2);
    nl_family_id = nla_get_u16(nla);

    printf("family id: %d\n", nl_family_id);
    set_family_id(nl_family_id);
}

int callback_message(struct nl_msg * msg, void * arg) {
    printf("received netlink message:\n");
    nl_msg_dump(msg, stdout);
    printf("received message end:\n");

    struct genlmsghdr* genl_hdr;
    int cmd;
    genl_hdr = (struct genlmsghdr *)nlmsg_data(nlmsg_hdr(msg));
    cmd = genl_hdr->cmd;

    if (cmd == CTRL_CMD_GETFAMILY){
        get_family_id(msg);
        set_director_pid(msg);
        printf("director peer port: %d\n", director_pid);
    }

    else if (cmd == DIRECTOR_REGISTER_PID){
        check_registered_director_pid(msg);
    }

    else if(cmd == DIRECTOR_NPM_RESPONSE){
        handle_npm_response(msg);
    }

    else if(cmd == DIRECTOR_IMMIGRATION_REQUEST_RESPONSE){
        handle_npm_immigration_request_response(msg);
    }
    else{
        printf("other unrecognized netlink message");
    }

    return 0;
}

void sig_handler(int signo)
{
    if (signo == SIGINT){
        printf("received SIGINT\n");
        close_connections();
        nl_socket_free(sk);
        exit(0);
    }
}

void should_run_process_cleaner(){
    run_process_cleaner = 1;    
}

void receive_netlink_message(){
    nl_recvmsgs_default(sk);
     
    struct nl_msg * msg;
    prepare_message(DIRECTOR_ACK, &msg);
    send_message(sk, msg);
    nlmsg_free(msg);
}

void try_netlink_receive(){
    printf("try netlink receive\n");
    fd_set socket_set;
    struct timeval tv;
    int fd = nl_socket_get_fd(sk);
    printf("netlink fd:%d\n", fd);
    FD_ZERO(&socket_set);
    FD_SET(fd, &socket_set);
    //wait max 0.5ms
    tv.tv_sec = 0;
    tv.tv_usec = 500;
    int ret;
    if ( (ret = select(fd + 1, &socket_set, NULL, NULL, &tv)) > 0){
        if(FD_ISSET(fd, &socket_set)){
            receive_netlink_message();
        }
    }
}


int main(){

	int ret_val = 0;
    struct nl_msg * msg;
    struct nlmsghdr *hdr;
    struct genlmsghdr genl_hdr;

    printf("Initializing clondike kernel simulator\n");

    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");

    init_ctlfs();
    

    sk = nl_socket_alloc();
    set_netlink_fd(sk); //set pointer in netlink_message file
	nl_socket_set_buffer_size(sk, 15000000, 15000000);

	nl_socket_disable_seq_check(sk);
	nl_socket_set_local_port(sk, LOCAL_NETLINK_PORT);

    genl_connect(sk);

    nl_socket_modify_cb(sk, NL_CB_MSG_IN, NL_CB_CUSTOM, callback_message, NULL);
	nl_recvmsgs_default(sk);
    printf("set peer port\n");
    nl_socket_set_peer_port(sk, director_pid);


    printf("ret genl_register: %d\n", genl_register_family(&my_genl_ops));

    printf("family registred\n");
    fflush(stdout);
    prepare_message(CTRL_CMD_NEWFAMILY, &msg);
    
    nla_put_string(msg, CTRL_ATTR_FAMILY_ID, "DIRECTORCHNL");

    send_message(sk, msg);

    nlmsg_free(msg);
    
    
    nl_recvmsgs_default(sk);

    prepare_message(DIRECTOR_ACK, &msg);
    send_message(sk, msg);

    nlmsg_free(msg);
   
    create_fifo();
    open_fifo();

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
        
        try_read_fifo();
        
        emig_send_messages();
        imig_send_messages();

        if (cycle%20 == 0){
            netlink_send_task_fork(get_next_pid(), get_next_pid());
        }
        ++cycle;
        printf("cycle: %d\n", cycle);

        if(run_process_cleaner){
            process_cleaner();
            run_process_cleaner = 0;
        }

        usleep(500000);

    }





    nl_socket_free(sk);
    
    genl_unregister_family(&my_genl_ops);
    //destroy_ctlfs();
   
    close_fifo();
    destroy_fifo();

    close_connections();

    return 0;
}

