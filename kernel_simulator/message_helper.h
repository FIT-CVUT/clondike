#ifndef MESSAGE_HELPER_H
#define MESSAGE_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <netlink/netlink.h>

unsigned int get_sequence_number();

void set_family_id(int family_id);

int prepare_message(uint8_t cmd, struct nl_msg ** res_msg);

int send_message(struct nl_sock *sk, struct nl_msg *msg);

#ifdef __cplusplus
}
#endif


#endif
