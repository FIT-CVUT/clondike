#include "message_helper.h"
#include "message_emigration_failed.h"
#include "msgs.h"


int send_emigration_failed(struct nl_sock * sk, int pid, const char * name, unsigned long jiffies){
    struct nl_msg *msg;
    int ret = 0;

    ret = prepare_emigration_failed(&msg, pid, name, jiffies);
    if (ret < 0){
        printf("cannot prepare message\n");
    }
    
    send_message(sk, msg);

    nlmsg_free(msg);

}

int prepare_emigration_failed(struct nl_msg ** ret_msg, int pid, const char * name, unsigned long jiffies){
    struct nl_msg *msg;
    int ret = 0;

    ret = prepare_message(DIRECTOR_EMIGRATION_FAILED, &msg);
    if(ret < 0){
        goto error;
    }

    ret = nla_put_u32(msg, DIRECTOR_A_PID, pid);
    if (ret < 0)
        goto error;

    ret = nla_put_string(msg, DIRECTOR_A_NAME, name);
    if (ret < 0)
        goto error;

    ret = nla_put_u64(msg, DIRECTOR_A_JIFFIES, jiffies);
    if (ret < 0)
        goto error;


    *ret_msg = msg;
    return 0;

error:
    nlmsg_free(msg);
    return ret;
}
