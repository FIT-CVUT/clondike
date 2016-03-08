#ifndef MESSAGE_HELPER_H
#define MESSAGE_HELPER_H

#include <netlink/netlink.h>

int prepare_message(uint8_t cmd, struct nl_msg ** res_msg);

int send_message(struct nl_sock *sk, struct nl_msg *msg);


#endif
