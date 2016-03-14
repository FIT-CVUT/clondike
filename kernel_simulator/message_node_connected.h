#ifndef MESSAGE_NODE_CONNECTED_H
#define MESSAGE_NODE_CONNECTED_H

#include <netlink/msg.h>

int send_node_connected(struct nl_sock * sk, const char * address, int index, const char * auth_data);

static int prepare_node_connected(struct nl_msg ** ret_msg, const char * address, int index, const char * auth_data);

#endif
