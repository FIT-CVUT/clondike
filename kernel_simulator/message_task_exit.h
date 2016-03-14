#ifndef MESSAGE_TASK_EXIT_H
#define MESSAGE_TASK_EXIT_H

#include <netlink/msg.h>

int send_task_exit(struct nl_sock * sk, int pid, int exit_code, int rusage);

int prepare_task_exit(struct nl_msg ** msg, int pid, int exit_code, int rusage);

#endif
