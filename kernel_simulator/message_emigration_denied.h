#ifndef MESSAGE_EMIGRATION_DENIED_H
#define MESSAGE_EMIGRATION_DENIED_H

#include <netlink/msg.h>

int send_emigration_denied(struct nl_sock * sk, int uid, int pid, int index, const char * name, unsigned long jiffies);
    
int prepare_emigration_denied(struct nl_msg ** ret_msg, int uid, int pid, int index, const char * name, unsigned long jiffies);

#endif