#ifndef KKC_H
#define KKC_H

#define MAX_DATA_LEN 1000

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

enum kkc_socket_types{
    KKC_EMIG_REQUEST,
    KKC_EMIG_REQUEST_RESPONSE,
    KKC_EMIG_BEGIN,
    KKC_EMIG_DONE,
    
    KKC_GENERIC_USER_MESSAGE
};

enum kkc_socket_attributes{
    ATTR_PID,
    ATTR_UID,
    ATTR_JIFFIES,
    ATTR_NAME,
    ATTR_RETURN_CODE,
    ATTR_DECISION,
    ATTR_SLOT_TYPE,
    ATTR_DATA,
    ATTR_DATA_LEN
};

struct kkc_message_header {
    uint16_t len;
    uint16_t type;
};

struct kkc_attr_header {
    uint16_t len;
    uint16_t type;
};

struct kkc_message{
    struct kkc_message_header hdr;
    char data[MAX_DATA_LEN];
};

struct kkc_attr{
    struct kkc_attr_header hdr;
    char data[MAX_DATA_LEN];
};


int start_ccn();

void try_receive_ccn();

void kkc_close_connections();

int get_address_from_file(const char * filename, struct sockaddr_in * server_address,  int clear);

int ccn_connect();

int kkc_receive_message(int fd, struct kkc_message ** ret_msg);

void kkc_handle_message(struct kkc_message * msg, int index);

int kkc_free_msg(struct kkc_message * msg);

struct kkc_attr * kkc_create_attr_u32(int type, uint32_t value);

struct kkc_attr * kkc_create_attr_u64(int type, uint64_t value);

struct kkc_attr * kkc_create_attr_string(int type, const char * value);

int kkc_send_all(int fd, const char * buf, int buf_len);

void kkc_dump_msg(const char * msg, int buflen);

int get_socket(int index);

void handle_emig_request_message(struct kkc_message * msg, int index);

void handle_emig_request_response_message(struct kkc_message * msg, int index);

void handle_emig_begin_message(struct kkc_message * msg, int index);

void handle_emig_done_message(struct kkc_message * msg, int index);

void handle_kkc_generic_message(struct kkc_message * msg, int index);

#ifdef __cplusplus
}
#endif

#endif
