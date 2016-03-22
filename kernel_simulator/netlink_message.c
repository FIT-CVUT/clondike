#include "netlink_message.h"
#include "message_emigration_failed.h"
#include "message_immigration_confirmed.h"
#include "message_immigration_request.h"
#include "message_node_connected.h"
#include "message_node_disconnected.h"
#include "message_npm_check.h"
#include "message_task_exit.h"
#include "message_task_fork.h"
#include "msgs.h"
#include "kkc_messages.h"

#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/genl/mngt.h>
#include <netlink/genl/genl.h>

static struct nl_sock* sk;

void set_netlink_fd(struct nl_sock * fd){
    sk = fd;
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
        imig_process_confirm(seq, MIGRATE);
    }

    return 0;
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

