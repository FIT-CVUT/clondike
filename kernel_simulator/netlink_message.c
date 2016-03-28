#include "netlink_message.h"
#include "message_emigration_failed.h"
#include "message_immigration_confirmed.h"
#include "message_immigration_request.h"
#include "message_node_connected.h"
#include "message_node_disconnected.h"
#include "message_npm_check.h"
#include "message_task_exit.h"
#include "message_task_fork.h"
#include "message_generic_user_message.h"
#include "msgs.h"
#include "response_handlers.h"
#include "kkc_messages.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/genl/mngt.h>
#include <netlink/genl/genl.h>

static int director_pid = 0;
static struct nl_sock* sk;

/*
struct genl_cmd my_genl_cmds[] = {
    {
        .c_id = DIRECTOR_ACK,
        .c_name = "DIRECTOR_ACK",
        .c_msg_parser = ack_handler,
    },
};
*/


struct genl_ops my_genl_ops = {
    .o_name = "DIRECTORCHNL",
    //.o_cmds = my_genl_cmds,
    .o_ncmds = 0,
};


int init_netlink(){
    printf("initializing netlink\n");

        struct nl_msg * msg;
    struct nlmsghdr *hdr;
    struct genlmsghdr genl_hdr;
    int ret = 0;

    sk = nl_socket_alloc();
	nl_socket_set_buffer_size(sk, 15000000, 15000000);

	nl_socket_disable_seq_check(sk);
	nl_socket_set_local_port(sk, LOCAL_NETLINK_PORT);

    genl_connect(sk);

    nl_socket_modify_cb(sk, NL_CB_MSG_IN, NL_CB_CUSTOM, netlink_callback_message, NULL);
	
    nl_recvmsgs_default(sk);
    printf("set peer port\n");
    nl_socket_set_peer_port(sk, director_pid);


    ret = genl_register_family(&my_genl_ops);
    if (ret < 0 ){
        printf("ERROR: cannot register netlink family\n");
        return ret;
    }

    printf("family registred\n");
    
    prepare_message(CTRL_CMD_NEWFAMILY, &msg);
    nla_put_string(msg, CTRL_ATTR_FAMILY_ID, "DIRECTORCHNL");
    send_message(sk, msg);
    nlmsg_free(msg);
    
    prepare_message(DIRECTOR_ACK, &msg);
    send_message(sk, msg);
    nlmsg_free(msg);
    
    //registering pid 
    nl_recvmsgs_default(sk);

    prepare_message(DIRECTOR_ACK, &msg);
    send_message(sk, msg);

    nlmsg_free(msg);
    
    return 0;
}

void netlink_close(){
    genl_unregister_family(&my_genl_ops);
    nl_socket_free(sk);
}

static void set_director_pid(struct nl_msg *msg){
    if (director_pid == 0){
        director_pid = nlmsg_hdr(msg)->nlmsg_pid;
    }
}

void try_netlink_receive(){
    //printf("try netlink receive\n");
    fd_set socket_set;
    struct timeval tv;
    int fd = nl_socket_get_fd(sk);
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

void receive_netlink_message(){
    nl_recvmsgs_default(sk);
     
    struct nl_msg * msg;
    prepare_message(DIRECTOR_ACK, &msg);
    send_message(sk, msg);
    nlmsg_free(msg);
}

int netlink_callback_message(struct nl_msg * msg, void * arg) {
    
#ifdef DEBUG
    printf("received netlink message:\n");
    nl_msg_dump(msg, stdout);
    printf("received message end:\n");
#endif

    struct genlmsghdr* genl_hdr;
    int cmd;
    genl_hdr = (struct genlmsghdr *)nlmsg_data(nlmsg_hdr(msg));
    cmd = genl_hdr->cmd;

    switch (cmd){
        case CTRL_CMD_GETFAMILY:
            get_family_id(msg);
            set_director_pid(msg);
            printf("director peer port: %d\n", director_pid);
            break;

        case DIRECTOR_REGISTER_PID:
            check_registered_director_pid(msg);
            break;
    
        case DIRECTOR_NPM_RESPONSE:
            handle_npm_response(msg);
            break;
    
        case DIRECTOR_IMMIGRATION_REQUEST_RESPONSE:
            handle_npm_immigration_request_response(msg);
            break;
    
        case DIRECTOR_SEND_GENERIC_USER_MESSAGE:
            handle_send_generic_user_message(msg);
            break;

        case DIRECTOR_NODE_CONNECT_RESPONSE:
            //TODO: do something
            //probably response for GENERIC_USER_MESSAGE
            break;
        case DIRECTOR_ACK:
            break;
        default:
            printf("other unrecognized netlink message\n");
    }

    return 0;
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

int handle_npm_response(struct nl_msg * msg){
    struct nlattr *nla;
    int decision, decision_value;

    unsigned int seq = nlmsg_hdr(msg)->nlmsg_seq;

    nla = nlmsg_find_attr(nlmsg_hdr(msg), sizeof(struct genlmsghdr), DIRECTOR_A_DECISION);
    if (nla == NULL)
        return -1;
    
    decision = nla_get_u32(nla);

    nla = nlmsg_find_attr(nlmsg_hdr(msg), sizeof(struct genlmsghdr), DIRECTOR_A_DECISION_VALUE);
    if (nla == NULL)
        return -1;
    
    decision_value = nla_get_u32(nla);

    if (decision == MIGRATE){
        printf("decision is MIGRATE, let's do it\n");
        emig_process_migrate(seq, decision_value);
    }
    else{
        emig_process_denied(seq);
    }

    return 0;
}

int handle_npm_immigration_request_response(struct nl_msg * msg){
    struct nlattr *nla;
    int decision, decision_value;

    unsigned int seq = nlmsg_hdr(msg)->nlmsg_seq;

    nla = nlmsg_find_attr(nlmsg_hdr(msg), sizeof(struct genlmsghdr), DIRECTOR_A_DECISION);
    if (nla == NULL)
        return -1;
    
    decision = nla_get_u32(nla);

    if (decision == MIGRATE){
        imig_process_confirm(seq, MIGRATE);
    }
    else{
        //TODO: maybe no branches needed
        imig_process_confirm(seq, DO_NOT_MIGRATE);
    }

    return 0;
}

int handle_send_generic_user_message(struct nl_msg * msg){
    struct nlattr *nla;
    int slot_index, slot_type, length;
    char * user_data;

    unsigned int seq = nlmsg_hdr(msg)->nlmsg_seq;

    nla = nlmsg_find_attr(nlmsg_hdr(msg), sizeof(struct genlmsghdr), DIRECTOR_A_SLOT_INDEX);
    if (nla == NULL)
        return -1;
    slot_index = nla_get_u32(nla);

    nla = nlmsg_find_attr(nlmsg_hdr(msg), sizeof(struct genlmsghdr), DIRECTOR_A_SLOT_TYPE);
    if (nla == NULL)
        return -1;
    slot_type = nla_get_u32(nla);
    
    nla = nlmsg_find_attr(nlmsg_hdr(msg), sizeof(struct genlmsghdr), DIRECTOR_A_LENGTH);
    if (nla == NULL)
        return -1;
    length = nla_get_u32(nla);
    
    nla = nlmsg_find_attr(nlmsg_hdr(msg), sizeof(struct genlmsghdr), DIRECTOR_A_USER_DATA);
    if (nla == NULL)
        return -1;
    user_data = nla_data(nla);

    kkc_send_generic_user_message(slot_type, slot_index, length, user_data);
}


int netlink_send_emigration_failed(int pid, const char * name, unsigned long jiffies){
    return send_emigration_failed(sk, pid, name, jiffies);
}

int netlink_send_immigration_confirmed(int uid, int pid, int index, const char * name, unsigned long jiffies, int remote_pid){
    return send_immigration_confirmed(sk, uid, pid, index, name, jiffies, remote_pid);
}

int netlink_send_immigration_request(int uid, int pid, int index, const char * name, unsigned long jiffies){
    return send_immigration_request(sk, uid, pid, index, name, jiffies);
}

int netlink_send_node_connected(struct sockaddr_in * pen_node_addr, int index){
    char address[50];
    sprintf(address, "%s:%d", inet_ntoa(pen_node_addr->sin_addr), ntohs(pen_node_addr->sin_port));

    return send_node_connected(sk, address, index, "");
}

int netlink_send_node_disconnected(int index, int slot_type, int reason){
    return send_node_disconnected(sk, index, slot_type, reason);
}

int netlink_send_npm_check(int pid, int uid, int task_type, const char * name, uint64_t jiffies, int rusage){
    return send_npm_check(sk, pid, uid, task_type, name, jiffies, rusage);
}

int netlink_send_npm_check_full(int pid, int uid, int task_type, const char * name, uint64_t jiffies, const char * const * argv, const char * const * envs){
    return send_npm_check_full(sk, pid, uid, task_type, name, jiffies, argv, envs);
}

int netlink_send_task_exit(int pid, int exit_code, int rusage){
    should_run_process_cleaner();
    return send_task_exit(sk, pid, exit_code, rusage);
}

int netlink_send_task_fork(int pid, int ppid){
    return send_task_fork(sk, pid, ppid);
}

int netlink_send_generic_user_message(int slot_index, int slot_type, int data_len, const char * data){
    return send_generic_user_message(sk, 0, slot_index, slot_type, data_len, data);

}

