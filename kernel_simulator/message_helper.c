#include "msgs.h"
#include "message_helper.h"


#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

static int family_id = 0;

void set_family_id(int fid){
    family_id = fid;
}

int prepare_message(uint8_t cmd, struct nl_msg ** res_msg){
    struct nl_msg *msg;
    struct nlmsghdr *hdr;

    struct genlmsghdr genl_hdr = {
        .cmd = cmd,
        .version = 0,
        .reserved = 0,
    };

    msg = nlmsg_alloc();
    if (msg == NULL){
        return -1;
    }
    
    genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, family_id, 0, 0, cmd, 0);

    *res_msg = msg;

    return 0;
}

int send_message(struct nl_sock *sk, struct nl_msg *msg){
    printf("Sending message\n");
    nl_msg_dump(msg, stdout);
    int ret;

    if ( (ret = nl_send_auto(sk, msg)) <= 0 ){
        return -1;
    }
    printf("Message succesfully send\n");

    return 0;
}
