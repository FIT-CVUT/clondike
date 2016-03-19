#ifndef MESSAGE_NODE_DISCONNECTED_H
#define MESSAGE_NODE_DISCONNECTED_H

#include <netlink/msg.h>

int send_node_disconnected(struct nl_sock * sk, int index, int slot_type, int reason);

static int prepare_node_disconnected(struct nl_msg ** ret_msg, int index, int slot_type, int reason);

#endif
