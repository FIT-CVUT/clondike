#include "message_helper.h"
#include "message_generic_user_message.h"
#include "msgs.h"

#include <netlink/attr.h>

int send_generic_user_message(struct nl_sock * sk, int index, int slot_index, int slot_type, int data_len, const char * data){
    struct nl_msg *msg;
    int ret = 0;

    ret = prepare_generic_user_message(&msg, index, slot_index, slot_type, data_len,  data);
    if (ret < 0){
        printf("cannot prepare message\n");
    }
    else
        printf("succesfuly prepared\n");
    
    send_message(sk, msg);

    nlmsg_free(msg);

}

int prepare_generic_user_message(struct nl_msg ** ret_msg, int index, int slot_index, int slot_type, int data_len, const char * data){
    struct nl_msg *msg;
    int ret = 0;

    ret = prepare_message(DIRECTOR_GENERIC_USER_MESSAGE, &msg);
    if(ret < 0){
        goto error;
    }

    ret = nla_put_u32(msg, DIRECTOR_A_INDEX, index);
    if (ret < 0)
        goto error;

    ret = nla_put_u32(msg, DIRECTOR_A_SLOT_INDEX, slot_index);
    if (ret < 0)
        goto error;

    ret = nla_put_u32(msg, DIRECTOR_A_SLOT_TYPE, slot_type);
    if (ret < 0)
        goto error;

    ret = nla_put_u32(msg, DIRECTOR_A_LENGTH, data_len);
    if (ret < 0)
        goto error;

    if (data_len){
        ret = nla_put(msg, DIRECTOR_A_USER_DATA, data_len, data);
        if (ret < 0)
            goto error;
    }

    *ret_msg = msg;
    return 0;

error:
    nlmsg_free(msg);
    return ret;
}


