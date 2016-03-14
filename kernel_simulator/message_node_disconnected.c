#include "message_helper.h"
#include "message_node_connected.h"
#include "msgs.h"

int send_node_disconnected(struct nl_sock * sk, int index, int slot_type, int reason){
    struct nl_msg *msg;
    int ret = 0;

    ret = prepare_node_disconnected(&msg, index, slot_type, reason);
    if (ret < 0){
        printf("cannot prepare message\n");
    }
    else
        printf("succesfuly prepared\n");

    send_message(sk, msg);

    nlmsg_free(msg);
}

static int prepare_node_connected(struct nl_msg ** ret_msg, int index, slot_type, int reason){
    struct nl_msg *msg;
    int ret = 0;
    int len = 0;

    ret = prepare_message(DIRECTOR_NODE_DISCONNECTED, &msg);
    if(ret < 0){
        goto error;
    }

    ret = nla_put_u32(msg, DIRECTOR_A_INDEX, index);
    if(ret < 0){
        goto error;
    }
    
    ret = nla_put_u32(msg, DIRECTOR_A_SLOT_TYPE, slot_type);
    if(ret < 0){
        goto error;
    }
    
    ret = nla_put_u32(msg, DIRECTOR_A_REASON, reason);
    if(ret < 0){
        goto error;
    }
    
    *ret_msg = msg;
    return 0;

error:
    nlmsg_free(msg);
    return ret;
}

