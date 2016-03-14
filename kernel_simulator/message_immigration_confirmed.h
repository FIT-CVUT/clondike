#ifndef MESSAGE_IMMIGRATION_CONFIRMED_H
#define MESSAGE_IMMIGRATION_CONFIRMED_H

#include <netlink/msg.h>

int send_immigration_confirmed(struct nl_sock * sk, int uid, int pid, int index, const char * name, unsigned long jiffies, int remote_pid);
    
int prepare_immigration_confirmed(struct nl_msg ** ret_msg, int uid, int pid, int index, const char * name, unsigned long jiffies, int remote_pid);

#endif
