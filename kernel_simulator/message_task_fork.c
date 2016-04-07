#include "message_helper.h"
#include "message_task_fork.h"
#include "msgs.h"


int send_task_fork(struct nl_sock * sk, int pid, int ppid){
    struct nl_msg *msg;
    int ret = 0;

    ret = prepare_task_fork(&msg, pid, ppid);
    if (ret < 0){
        printf("cannot prepare message\n");
    }
    
    send_message(sk, msg);

    nlmsg_free(msg);

}

int prepare_task_fork(struct nl_msg ** ret_msg, int pid, int ppid){
    struct nl_msg *msg;
    int ret = 0;

    ret = prepare_message(DIRECTOR_TASK_FORK, &msg);
    if(ret < 0){
        goto error;
    }

    ret = nla_put_u32(msg, DIRECTOR_A_PID, pid);
    if (ret < 0)
        goto error;

    ret = nla_put_u32(msg, DIRECTOR_A_PPID, ppid);
    if (ret < 0)
        goto error;

    *ret_msg = msg;
    return 0;

error:
    nlmsg_free(msg);
    return ret;
}
