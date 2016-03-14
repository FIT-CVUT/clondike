#ifndef MESSAGE_TASK_FORKED_H
#define MESSAGE_TASK_FORKED_H

#include <netlink/msg.h>

int send_task_forked(struct nl_sock * sk, int pid, int ppid);

int prepare_task_forked(struct nl_msg ** msg, int pid, int ppid);

#endif
