#ifndef KKC_MESSAGES_H
#define KKC_MESSAGES_H

#ifdef __cplusplus
extern "C" {
#endif


int kkc_send_emig_request(int peer_message, int pid, int uid, const char * name);

int kkc_send_emig_request_response(int peer_message, int pid, int decision);

int kkc_send_emig_begin(int peer_message, int pid, int uid, const char * name);

int kkc_send_emig_done(int peer_message, int pid, int return_code);

#ifdef __cplusplus
}
#endif

#endif
