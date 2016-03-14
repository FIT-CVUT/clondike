#include "ctlfs.h"
#include "message_helper.h"
#include "msgs.h"
#include "response_handlers.h"
#include "message_task_fork.h"
#include "message_task_exit.h"
#include "message_node_connected.h"

#include <stdio.h>
#include <stdlib.h>

#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/genl/mngt.h>
#include <netlink/genl/genl.h>

#define LOCAL_NETLINK_PORT 111111111

int director_pid = 0;

struct genl_cmd my_genl_cmds[] = {
    {
        .c_id = DIRECTOR_ACK,
        .c_name = "DIRECTOR_ACK",
        .c_msg_parser = ack_handler,
    }
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
    printf("received message:\n");
    nl_msg_dump(msg, stdout);
    printf("received message end:\n");

    struct genlmsghdr* genl_hdr;
    int cmd;
    genl_hdr = (struct genlmsghdr *)nlmsg_data(nlmsg_hdr(msg));
    cmd = genl_hdr->cmd;

    if (cmd == CTRL_CMD_GETFAMILY){
        get_family_id(msg);
    }

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
	nl_socket_set_local_port(sk, LOCAL_NETLINK_PORT);

    genl_connect(sk);

    nl_socket_modify_cb(sk, NL_CB_MSG_IN, NL_CB_CUSTOM, callback_message, NULL);
	nl_recvmsgs_default(sk);
    printf("set peer port\n");
    nl_socket_set_peer_port(sk, director_pid);


    printf("ret genl_register: %d\n", genl_register_family(&my_genl_ops));

    /*
    if (genl_ops_resolve(sk, &my_genl_ops) < 0)
        printf( "Unable to resolve family name\n");
*/

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




//test
    printf("send task fork\n");
    send_task_fork(sk, 111,1111);
    nl_recvmsgs_default(sk);



    printf("send task exit\n");
    send_task_exit(sk, 111, 0, 0);
    nl_recvmsgs_default(sk);

    printf("sending npm_check simple\n");
    send_npm_check(sk, 1234, 1000, 0, "/home/clondike/nic", 1123123213123123, 1);
    nl_recvmsgs_default(sk);

    const char *argv[] = {
        "jedna", "dva", NULL
    };

    const char *envp[] = {
        "path1", "path2", NULL
    };

    send_npm_check_full(sk, 1234, 1000, 0, "/usr/bin/fridlik", 41234, argv, envp);
    nl_recvmsgs_default(sk);

    send_node_connected(sk, "192.168.21.133:48123", 0, "");
    nl_recvmsgs_default(sk);
    nl_recvmsgs_default(sk);

    prepare_message(DIRECTOR_ACK, &msg);
    send_message(sk, msg);
    nlmsg_free(msg);

    sleep(10);


    send_node_connected(sk, "192.168.21.132:58123", 1, "");
    nl_recvmsgs_default(sk);

    printf("sending npm_check simple\n");
    send_npm_check(sk, 1245, 100, 0, "/dva/tri", 112312323123, 0);
    nl_recvmsgs_default(sk);




    nl_socket_free(sk);
    
    genl_unregister_family(&my_genl_ops);
    //destroy_ctlfs();

    return 0;
}

