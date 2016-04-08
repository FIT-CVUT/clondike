#ifndef KKC_MESSAGES_H
#define KKC_MESSAGES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

int kkc_send_emig_request(int peer_message, int pid, int uid, const char * name, uint64_t jiffies);

int kkc_send_emig_request_response(int peer_message, int pid, int decision);

int kkc_send_emig_begin(int peer_message, int pid, int uid, const char * name);

int kkc_send_emig_done(int peer_message, int pid, int return_code);

int kkc_send_generic_user_message(int slot_type, int slot_index, int data_len, const char * data);

#ifdef __cplusplus
}
#endif

#endif
