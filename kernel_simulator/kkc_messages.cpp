#include "kkc_messages.h"
#include "kkc.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;


int kkc_send_emig_request(int peer_index, int pid, int uid, const char * name){
    int fd = get_socket(peer_index);
    if(fd == -1)
        return -1;

    struct kkc_message_header hdr;
    hdr.len = sizeof(struct kkc_message_header); //need to be updated later
    hdr.type = KKC_EMIG_REQUEST;


    struct kkc_attr * attr_pid = kkc_create_attr_u32(ATTR_PID, pid);
    struct kkc_attr * attr_uid = kkc_create_attr_u32(ATTR_UID, uid);
    struct kkc_attr * attr_name = kkc_create_attr_string(ATTR_NAME, name);

    hdr.len += attr_pid->hdr.len;
    hdr.len += attr_uid->hdr.len;
    hdr.len += attr_name->hdr.len;

    char * buf = (char *) malloc(sizeof(char) * hdr.len);
    int position = 0;

    memcpy(buf+position, &hdr, sizeof(struct kkc_message_header));
    position += sizeof(kkc_message_header);

    memcpy(buf+position, (char *)attr_pid, attr_pid->hdr.len);
    position += attr_pid->hdr.len;

    memcpy(buf+position, attr_uid, attr_uid->hdr.len);
    position += attr_uid->hdr.len;

    memcpy(buf+position, attr_name, attr_name->hdr.len);
    position += attr_name->hdr.len;

#ifdef DEBUG
    cout << "sending message, len: " << hdr.len << endl;
    kkc_dump_msg(buf, hdr.len);
#endif

    if (kkc_send_all(fd, buf, hdr.len) != hdr.len){
        cout << "sending error" << endl;
        return -1;
    }
    cout << "message successfuly send" << endl;
    free(buf);
    free(attr_pid);
    free(attr_uid);
    free(attr_name);
    return 0;
}

int kkc_send_emig_request_response(int peer_index, int pid, int decision){
    int fd = get_socket(peer_index);
    if(fd == -1)
        return -1;

    struct kkc_message_header hdr;
    hdr.len = sizeof(struct kkc_message_header); //need to be updated later
    hdr.type = KKC_EMIG_REQUEST_RESPONSE;

    struct kkc_attr * attr_pid = kkc_create_attr_u32(ATTR_PID, pid);
    struct kkc_attr * attr_decision = kkc_create_attr_u32(ATTR_DECISION, decision);

    hdr.len += attr_pid->hdr.len;
    hdr.len += attr_decision->hdr.len;

    char * buf = (char *) malloc(sizeof(char) * hdr.len);
    int position = 0;

    memcpy(buf+position, &hdr, sizeof(struct kkc_message_header));
    position += sizeof(kkc_message_header);

    memcpy(buf+position, (char *)attr_pid, attr_pid->hdr.len);
    position += attr_pid->hdr.len;

    memcpy(buf+position, (char *)attr_decision, attr_decision->hdr.len);

    cout << "sending emig_request_respose, len: " << hdr.len << endl;
    kkc_dump_msg(buf, hdr.len);

    if (kkc_send_all(fd, buf, hdr.len) != hdr.len){
        cout << "sending error" << endl;
        return -1;
    }
    cout << "message successfuly send" << endl;
    free(buf);
    free(attr_pid);
    free(attr_decision);
    return 0;
}

int kkc_send_emig_begin(int peer_index, int pid, int uid, const char * name){
    int fd = get_socket(peer_index);
    if(fd == -1)
        return -1;

    struct kkc_message_header hdr;
    hdr.len = sizeof(struct kkc_message_header); //need to be updated later
    hdr.type = KKC_EMIG_BEGIN;


    struct kkc_attr * attr_pid = kkc_create_attr_u32(ATTR_PID, pid);
    struct kkc_attr * attr_uid = kkc_create_attr_u32(ATTR_UID, uid);
    struct kkc_attr * attr_name = kkc_create_attr_string(ATTR_NAME, name);

    hdr.len += attr_pid->hdr.len;
    hdr.len += attr_uid->hdr.len;
    hdr.len += attr_name->hdr.len;

    char * buf = (char *) malloc(sizeof(char) * hdr.len);
    int position = 0;

    memcpy(buf+position, &hdr, sizeof(struct kkc_message_header));
    position += sizeof(kkc_message_header);

    memcpy(buf+position, (char *)attr_pid, attr_pid->hdr.len);
    position += attr_pid->hdr.len;

    memcpy(buf+position, attr_uid, attr_uid->hdr.len);
    position += attr_uid->hdr.len;

    memcpy(buf+position, attr_name, attr_name->hdr.len);
    position += attr_name->hdr.len;

#ifdef DEBUG
    cout << "sending emig_begin, len: " << hdr.len << endl;
    kkc_dump_msg(buf, hdr.len);
#endif

    if (kkc_send_all(fd, buf, hdr.len) != hdr.len){
        cout << "sending error" << endl;
        return -1;
    }
    cout << "message successfuly send" << endl;
    free(buf);
    free(attr_pid);
    free(attr_uid);
    free(attr_name);
    return 0;
}

int kkc_send_emig_done(int peer_index, int pid, int return_code){
    int fd = get_socket(peer_index);
    if(fd == -1)
        return -1;

    struct kkc_message_header hdr;
    hdr.len = sizeof(struct kkc_message_header); //need to be updated later
    hdr.type = KKC_EMIG_DONE;

    struct kkc_attr * attr_pid = kkc_create_attr_u32(ATTR_PID, pid);
    struct kkc_attr * attr_ret = kkc_create_attr_u32(ATTR_RETURN_CODE, return_code);

    hdr.len += attr_pid->hdr.len;
    hdr.len += attr_ret->hdr.len;

    char * buf = (char *) malloc(sizeof(char) * hdr.len);
    int position = 0;

    memcpy(buf+position, &hdr, sizeof(struct kkc_message_header));
    position += sizeof(kkc_message_header);

    memcpy(buf+position, (char *)attr_pid, attr_pid->hdr.len);
    position += attr_pid->hdr.len;

    memcpy(buf+position, attr_ret, attr_ret->hdr.len);
    position += attr_ret->hdr.len;

#ifdef DEBUG
    cout << "sending emig_done, len: " << hdr.len << endl;
    kkc_dump_msg(buf, hdr.len);
#endif

    if (kkc_send_all(fd, buf, hdr.len) != hdr.len){
        cout << "sending error" << endl;
        return -1;
    }
    cout << "message successfuly send" << endl;
    free(buf);
    free(attr_pid);
    free(attr_ret);
    return 0;
}

int kkc_send_generic_user_message(int slot_type, int slot_index, int data_len, const char * data){
    int fd = get_socket(slot_index);
    if(fd == -1)
        return -1;

    struct kkc_message_header hdr;
    hdr.len = sizeof(struct kkc_message_header);
    hdr.type = KKC_GENERIC_USER_MESSAGE;

    struct kkc_attr * attr_slot_type = kkc_create_attr_u32(ATTR_PID, slot_type);
    struct kkc_attr * attr_data_len = kkc_create_attr_u32(ATTR_PID, data_len);
    struct kkc_attr * attr_data = kkc_create_attr_string(ATTR_NAME, data);

    hdr.len += attr_slot_type->hdr.len;
    hdr.len += attr_data_len->hdr.len;
    hdr.len += attr_data->hdr.len;

    char * buf = (char *) malloc(sizeof(char) * hdr.len);
    int position = 0;
    
    //hdr
    memcpy(buf+position, &hdr, sizeof(struct kkc_message_header));
    position += sizeof(kkc_message_header);

    //slot_type
    memcpy(buf+position, attr_slot_type, attr_slot_type->hdr.len);
    position += attr_slot_type->hdr.len;

    //data_len
    memcpy(buf+position, attr_data_len, attr_data_len->hdr.len);
    position += attr_data_len->hdr.len;

    //data
    memcpy(buf+position, attr_data, attr_data->hdr.len);
    position += attr_data->hdr.len;

#ifdef DEBUG
    cout << "sending KKC emig_generic_user_message, len: " << hdr.len << endl;
    kkc_dump_msg(buf, hdr.len);
#endif
    if (kkc_send_all(fd, buf, hdr.len) != hdr.len){
        cout << "sending error" << endl;
        return -1;
    }
    cout << "message successfuly send" << endl;
    free(buf);
    free(attr_slot_type);
    free(attr_data_len);
    free(attr_data);
    return 0;
}
