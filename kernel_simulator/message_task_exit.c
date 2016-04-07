#include "message_helper.h"
#include "message_task_exit.h"
#include "msgs.h"


int send_task_exit(struct nl_sock * sk, int pid, int exit_code, int rusage){
    struct nl_msg *msg;
    int ret = 0;

    printf("task exit\n");
    ret = prepare_task_exit(&msg, pid, exit_code, rusage);
    if (ret < 0){
        printf("cannot prepare message\n");
    }
    
    send_message(sk, msg);

    nlmsg_free(msg);

}

int prepare_task_exit(struct nl_msg ** ret_msg, int pid, int exit_code, int rusage){
    struct nl_msg *msg;
    int ret = 0;

    ret = prepare_message(DIRECTOR_TASK_EXIT, &msg);
    if(ret < 0){
        goto error;
    }

    ret = nla_put_u32(msg, DIRECTOR_A_PID, pid);
    if (ret < 0)
        goto error;

    ret = nla_put_u32(msg, DIRECTOR_A_EXIT_CODE, exit_code);
    if (ret < 0)
        goto error;

    ret = nla_put_u32(msg, DIRECTOR_A_RUSAGE, rusage);
    if (ret < 0)
        goto error;


    *ret_msg = msg;
    return 0;

error:
    nlmsg_free(msg);
    return ret;
}
