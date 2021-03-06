#include "npm_msg.h"
#include "msgs.h"
#include "genl_ext.h"
#include "comm.h"

#include "task_forked_msg.h"

#include <dbg.h>

#include <linux/skbuff.h>

struct task_forked_params {
	/* In params */
	u32 pid;
	u32 ppid;

	/* Out params -> NONE */

};

static int task_forked_create_request(struct sk_buff *skb, void* params) {
  	int ret = 0;
  	struct task_forked_params* task_forked_params = params;

	ret = nla_put_u32(skb, DIRECTOR_A_PID, task_forked_params->pid);
  	if (ret != 0)
      		goto failure;

	ret = nla_put_u32(skb, DIRECTOR_A_PPID, task_forked_params->ppid);
  	if (ret != 0)
      		goto failure;

failure:
	return ret;
}


static int task_forked_read_response(struct genl_info* info, void* params) {
	int ret = 0;
//	struct task_exitted_params* task_exitted_params = params;

	return ret;
}

static struct msg_transaction_ops task_forked_msg_ops = {
	.create_request = task_forked_create_request,
	.read_response = task_forked_read_response
};


int task_forked(pid_t pid, pid_t ppid) {
	struct task_forked_params params;
	int ret = 0;

	params.pid = pid;
	params.ppid = ppid;

	ret = msg_transaction_do(DIRECTOR_TASK_FORK, &task_forked_msg_ops, &params, 1);

	minfo(INFO3, "Task forked. Pid:  %u -> %u -> Res: %d", ppid, pid, ret);

	return ret;
}
