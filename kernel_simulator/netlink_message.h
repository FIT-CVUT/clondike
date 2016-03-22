#ifndef NNETLINK_MESSAGE_H
#define NNETLINK_MESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/genl/mngt.h>
#include <netlink/genl/genl.h>

#define LOCAL_NETLINK_PORT 111111111

enum npm_msg_response {
    DO_NOT_MIGRATE,
    MIGRATE, // In this case second return params is target migman id
    MIGRATE_BACK,
    // Following return codes indicate, that we need to provide more info to user mode director for decision
    REQUIRE_ARGS,
    REQUIRE_ENVP,
    REQUIRE_ARGS_AND_ENVP
};


int init_netlink();

void receive_netlink_message();

void get_family_id(struct nl_msg *msg);

int netlink_callback_message(struct nl_msg * msg, void * arg);

int netlink_send_emigration_failed(int pid, const char * name, unsigned long jiffies);

int netlink_send_immigration_confirmed(int uid, int pid, int index, const char * name, unsigned long jiffies, int remote_pid);

int netlink_send_immigration_request(int uid, int pid, int index, const char * name, unsigned long jiffies);

int netlink_send_node_connected(struct sockaddr_in * pen_node_addr, int index);

int netlink_send_node_disconnected(int index, int slot_type, int reason);

int netlink_send_npm_check(int pid, int uid, int task_type, const char * name, uint64_t jiffies, int rusage);

int netlink_send_npm_check_full(int pid, int uid, int task_type, const char * name, uint64_t jiffies, const char * const * argv, const char * const * envs);

int netlink_send_task_exit(int pid, int exit_code, int rusage);

int netlink_send_task_fork(int pid, int ppid);

int handle_npm_response(struct nl_msg * msg);

int handle_npm_immigration_request_response(struct nl_msg * msg);


#ifdef __cplusplus
}
#endif

#endif
