#include "message_helper.h"
#include "message_npm_check.h"
#include "msgs.h"

#include <netlink/attr.h>

int send_npm_check(struct nl_sock * sk, int pid, int uid, int task_type, const char * name, uint64_t jiffies, int rusage){
    struct nl_msg *msg;
    int ret = 0;

    ret = prepare_npm_check(&msg, pid, uid, task_type, name, jiffies, rusage);
    if (ret < 0){
        printf("cannot prepare message\n");
    }
    else
        printf("succesfuly prepared\n");
    
    send_message(sk, msg);

    nlmsg_free(msg);

}

int prepare_npm_check(struct nl_msg ** ret_msg, int pid, int uid, int task_type, const char * name, uint64_t jiffies, int rusage){
    struct nl_msg *msg;
    int ret = 0;

    ret = prepare_message(DIRECTOR_CHECK_NPM, &msg);
    if(ret < 0){
        goto error;
    }

    ret = nla_put_u32(msg, DIRECTOR_A_PID, pid);
    if (ret < 0)
        goto error;

    ret = nla_put_u32(msg, DIRECTOR_A_UID, uid);
    if (ret < 0)
        goto error;

    ret = nla_put_u32(msg, DIRECTOR_A_TASK_TYPE, task_type);
    if (ret < 0)
        goto error;

    ret = nla_put_string(msg, DIRECTOR_A_NAME, name);
    if (ret < 0)
        goto error;

    ret = nla_put_u64(msg, DIRECTOR_A_JIFFIES, jiffies);
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


static int puts_nested(struct nl_msg *msg, int type, int nested_type, const char * const * arg){

    struct nlattr *opts;
    int ret = 0;
    int i;

    if (!arg){
        return 0;
    }
    if(!(opts = nla_nest_start(msg, type))){
        nla_nest_cancel(msg, opts);
        return -1;
    }

    for (i = 0; arg[i]; i++){
        if (nla_put_string(msg, nested_type, arg[i]) < 0){
            printf("Cannot insert nested attribute\n");
        }
    }

    ret = nla_put_u32(msg, DIRECTOR_A_LENGTH, i);
    if (ret < 0)
        return -2;

    nla_nest_end(msg, opts);
#ifdef DEBUG
    printf("Nested args successfuly inserted\n");
#endif

    return 0;
}

int send_npm_check_full(struct nl_sock * sk, int pid, int uid, int task_type, const char * name, uint64_t jiffies, const char * const * argv, const char * const * envs){
    struct nl_msg *msg;
    int ret = 0;

    ret = prepare_npm_check_full(&msg, pid, uid, task_type, name, jiffies, argv, envs);
    if (ret < 0){
        printf("cannot prepare message\n");
    }
    else
        printf("succesfuly prepared\n");

    send_message(sk, msg);

    nlmsg_free(msg);

}


int prepare_npm_check_full(struct nl_msg ** ret_msg, int pid, int uid, int task_type, const char * name, uint64_t jiffies, const char * const * argv, const char * const * envs){
    struct nl_msg *msg;
    int ret = 0;

    ret = prepare_message(DIRECTOR_CHECK_FULL_NPM, &msg);
    if(ret < 0){
        goto error;
    }

    ret = nla_put_u32(msg, DIRECTOR_A_PID, pid);
    if (ret < 0)
        goto error;

    ret = nla_put_u32(msg, DIRECTOR_A_UID, uid);
    if (ret < 0)
        goto error;

    ret = nla_put_u32(msg, DIRECTOR_A_TASK_TYPE, task_type);
    if (ret < 0)
        goto error;

    ret = nla_put_string(msg, DIRECTOR_A_NAME, name);
    if (ret < 0)
        goto error;

    ret = nla_put_u64(msg, DIRECTOR_A_JIFFIES, jiffies);
    if (ret < 0)
        goto error;

    ret = puts_nested(msg, DIRECTOR_A_ARGS, DIRECTOR_A_ARG, argv);
    if (ret < 0)
        goto error;
    
    ret = puts_nested(msg, DIRECTOR_A_ENVS, DIRECTOR_A_ENV, envs);
    if (ret < 0)
        goto error;


    *ret_msg = msg;
    return 0;

error:
    nlmsg_free(msg);
    return ret;
}

