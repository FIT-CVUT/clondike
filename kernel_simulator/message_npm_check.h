#ifndef MESSAGE_NPM_CHECK_H
#define MESSAGE_NPM_CHECK_H

int send_npm_check(struct nl_sock * sk, int pid, int uid, int task_type, const char * name, uint64_t jiffies, int rusage);

int prepare_npm_check(struct nl_msg ** ret_msg, int pid, int uid, int task_type, const char * name, uint64_t jiffies, int rusage);

static int puts_nested(struct nl_msg *msg, int type, int nested_type, const char * const * arg);

int prepare_npm_check_full(struct nl_msg ** ret_msg, int pid, int uid, int task_type, const char * name, uint64_t jiffies, const char * const * argv, const char * const * envs);

int send_npm_check_full(struct nl_sock * sk, int pid, int uid, int task_type, const char * name, uint64_t jiffies, const char * const * argv, const char * const * envs);

#endif
