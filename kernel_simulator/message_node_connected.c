#include "message_helper.h"
#include "message_node_connected.h"
#include "msgs.h"

int send_node_connected(struct nl_sock * sk, const char * address, int index, const char * auth_data){
    struct nl_msg *msg;
    int ret = 0;

    ret = prepare_node_connected(&msg, address, index, auth_data);
    if (ret < 0){
        printf("cannot prepare message\n");
    }
    else
        printf("succesfuly prepared\n");

    send_message(sk, msg);

    nlmsg_free(msg);
}

static int prepare_node_connected(struct nl_msg ** ret_msg, const char * address, int index, const char * auth_data){
    struct nl_msg *msg;
    int ret = 0;
    int len = 0;

    ret = prepare_message(DIRECTOR_NODE_CONNECTED, &msg);
    if(ret < 0){
        goto error;
    }

    ret = nla_put_string(msg, DIRECTOR_A_ADDRESS, address);
    if(ret < 0){
        goto error;
    }

    ret = nla_put_u32(msg, DIRECTOR_A_INDEX, index);
    if(ret < 0){
        goto error;
    }
    
    
    if (auth_data){
        len = strlen(auth_data);
    }
    
    ret = nla_put_u32(msg, DIRECTOR_A_LENGTH, len);
    if(ret < 0){
        goto error;
    }

    if (len){
        ret = nla_put(msg, DIRECTOR_A_AUTH_DATA, len, auth_data);
        if(ret < 0){
            goto error;
        }
    }

    *ret_msg = msg;
    return 0;

error:
    nlmsg_free(msg);
    return ret;
}

