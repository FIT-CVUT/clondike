#include "emigration-denied.h"
#include "director-api.h"
#include "msg-common.h"
#include "msgs.h"
#include "internal.h"

#include <errno.h>

static emigration_denied_callback_t emigration_denied_callback = NULL;

void register_emigration_denied_callback(emigration_denied_callback_t callback) {
	emigration_denied_callback = callback;
}

int handle_emigration_denied(struct nl_msg *req_msg) {
	struct nl_msg *msg = NULL;
	struct nlattr *nla;
	int ret = 0;
	int seq;
	struct internal_state* state = get_current_state();

	// In params	
	int uid;
	pid_t pid;
	int slot_index;
	char* name;
	unsigned long jiffies;
	// Out params
	int accept = 1;
	
	seq = nlmsg_hdr(req_msg)->nlmsg_seq;

	nla = nlmsg_find_attr(nlmsg_hdr(req_msg), sizeof(struct genlmsghdr), DIRECTOR_A_UID);
	if (nla == NULL)
		return  -EBADMSG;
	uid = nla_get_u32(nla);

	nla = nlmsg_find_attr(nlmsg_hdr(req_msg), sizeof(struct genlmsghdr), DIRECTOR_A_PID);
	if (nla == NULL)
		return  -EBADMSG;
	pid = nla_get_u32(nla);

	nla = nlmsg_find_attr(nlmsg_hdr(req_msg), sizeof(struct genlmsghdr), DIRECTOR_A_INDEX);
	if (nla == NULL)
		return  -EBADMSG;
	slot_index = nla_get_u32(nla);

	nla = nlmsg_find_attr(nlmsg_hdr(req_msg), sizeof(struct genlmsghdr), DIRECTOR_A_NAME);
	if (nla == NULL)
		return  -EBADMSG;
	//name = nl_data_get(nla_get_data(nla));
	name = nla_data(nla);

	nla = nlmsg_find_attr(nlmsg_hdr(req_msg), sizeof(struct genlmsghdr), DIRECTOR_A_JIFFIES);
	if (nla == NULL)
		return  -EBADMSG;
	jiffies = nla_get_u64(nla);

	//printf("NPM CALLED FOR NAME: %s\n", name);
	if ( emigration_denied_callback )
        	emigration_denied_callback(uid, pid, slot_index, name, jiffies, &accept);
	
	if ( (ret=prepare_response_message(state->sk, DIRECTOR_ACK, state->gnl_fid, seq, &msg) ) != 0 ) {
		goto done;
	}
	
	ret = nla_put_u32(msg,
			DIRECTOR_A_DECISION,
			accept);

	if (ret != 0)
		goto error_del_resp;

	ret = send_request_message(state->sk, msg, 0);
	goto done;	

error_del_resp:
	nlmsg_free(msg);
done:	
	return ret;
}
