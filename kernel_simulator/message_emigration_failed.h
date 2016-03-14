#ifndef MESSAGE_EMIGRATION_FAILED_H
#define MESSAGE_EMIGRATION_FAILED_H

#include <netlink/msg.h>

int send_emigration_failed(struct nl_sock * sk, int pid, const char * name, unsigned long jiffies);

int prepare_emigration_failed(struct nl_msg ** ret_msg, int pid, const char * name, unsigned long jiffies);
      
#endif
