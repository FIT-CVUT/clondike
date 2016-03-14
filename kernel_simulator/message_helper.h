#ifndef MESSAGE_HELPER_H
#define MESSAGE_HELPER_H

#include <netlink/netlink.h>

void set_family_id(int family_id);

int prepare_message(uint8_t cmd, struct nl_msg ** res_msg);

int send_message(struct nl_sock *sk, struct nl_msg *msg);


#endif
