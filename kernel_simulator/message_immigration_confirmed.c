#include "message_helper.h"
#include "message_immigration_confirmed.h"
#include "msgs.h"


int send_immigration_confirmed(struct nl_sock * sk, int uid, int pid, int index, const char * name, unsigned long jiffies, int remote_pid){
    struct nl_msg *msg;
    int ret = 0;

    ret = prepare_immigration_confirmed(&msg, uid, pid, index, name, jiffies, remote_pid);
    if (ret < 0){
        printf("cannot prepare message\n");
    }
    
    send_message(sk, msg);

    nlmsg_free(msg);
    
    return 0;
}

int prepare_immigration_confirmed(struct nl_msg ** ret_msg, int uid, int pid, int index, const char * name, unsigned long jiffies, int remote_pid){
    struct nl_msg *msg;
    int ret = 0;

    ret = prepare_message(DIRECTOR_IMMIGRATION_CONFIRMED, &msg);
    if(ret < 0){
        goto error;
    }

    ret = nla_put_u32(msg, DIRECTOR_A_UID, uid);
    if (ret < 0)
        goto error;

    ret = nla_put_u32(msg, DIRECTOR_A_PID, pid);
    if (ret < 0)
        goto error;

    ret = nla_put_u32(msg, DIRECTOR_A_INDEX, index);
    if (ret < 0)
        goto error;

    ret = nla_put_string(msg, DIRECTOR_A_NAME, name);
    if (ret < 0)
        goto error;

    ret = nla_put_u64(msg, DIRECTOR_A_JIFFIES, jiffies);
    if (ret < 0)
        goto error;

    ret = nla_put_u32(msg, DIRECTOR_A_REMOTE_PID, remote_pid);
    if (ret < 0)
        goto error;


    *ret_msg = msg;
    return 0;

error:
    nlmsg_free(msg);
    return ret;
}
