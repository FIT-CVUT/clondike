#ifndef MESSAGE_IMMIGRATION_REQUEST_H
#define MESSAGE_IMMIGRATION_REQUEST_H

#include <netlink/msg.h>

int send_immigration_request(struct nl_sock * sk, int uid, int pid, int index, const char * name, unsigned long jiffies);
    
int prepare_immigration_request(struct nl_msg ** ret_msg, int uid, int pid, int index, const char * name, unsigned long jiffies);

#endif
