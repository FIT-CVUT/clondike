#include "msgs.h"
#include "comm.h"
#include "genl_ext.h"

#include <linux/skbuff.h>
#include <linux/resource.h>
#include <dbg.h>

#include "generic_user_message_send_handler.h"

const char* DIRECTOR_CHANNEL_NAME = "DIRECTORCHNL";

/** Pid of associated user space process (if any) */
static u32 user_director_pid = 0;
/** Sequence generating unique transaction numbers */
static atomic_t director_seq = ATOMIC_INIT(1);

/** Read timeout in seconds */
static int read_timeout = 2;

static struct genl_family director_gnl_family = {
	.id = GENL_ID_GENERATE,
	.hdrsize = 0,
	.name = "DIRECTORCHNL", /*DIRECTOR_CHANNEL_NAME... MUST BE SHORTER THAN 16 CHARS!!!!!!!!!!!!!!!!! */
	.version = 1,
	.maxattr = DIRECTOR_ATTR_MAX
};

/* Attribute policies */
static struct nla_policy director_genl_policy[DIRECTOR_ATTR_MAX + 1] = {
	[DIRECTOR_A_PID] = { .type = NLA_U32 },
	[DIRECTOR_A_LENGTH] = { .type = NLA_U32 },
	[DIRECTOR_A_EXIT_CODE] = { .type = NLA_U32 },
	[DIRECTOR_A_ERRNO] = { .type = NLA_U32 },
	[DIRECTOR_A_RUSAGE] = { .type = NLA_BINARY, .len = sizeof(struct rusage) },
};

/**
 * Checks, if the transaction returned an error.
 *
 * @return 0, if there was no error, error code otherwise
 */
static int check_for_error(struct genl_info* info) {
	struct nlattr* attr;
	int err;

	attr = nlmsg_find_attr(info->nlhdr, sizeof(struct genlmsghdr), DIRECTOR_A_ERRNO);
	if ( attr == NULL ) {
		return 0;
	}

	err = nla_get_u32(attr); 
	minfo(ERR3, "Error code from user deamon: %d", err);
	return err;
}

/** Returns pid of director process if there is user-space director connected, 0 otherwise */
static int is_director_connected(void) {
	return user_director_pid;
}

int is_director_pid(pid_t ppid) {
	return (u32)ppid == user_director_pid;
}

pid_t get_director_pid(void) {
	return user_director_pid;
}

void disconnect_director(void){
	user_director_pid = 0;
}

/** Returns unique sequence number for transaction */
static int get_unique_seq(void) {
	return atomic_inc_return(&director_seq);
}

/* Callback for initial daemon registration */
static int register_pid_handler(struct sk_buff *skb, struct genl_info *info) {
	struct nlattr* attr;
	minfo(INFO4, "Registering pid info:");
	mdbg(INFO4, "Registering pid info:");

	attr = nlmsg_find_attr(info->nlhdr, sizeof(struct genlmsghdr), DIRECTOR_A_PID);
	if ( attr == NULL ) {
		return -1;
	}

	user_director_pid = nla_get_u32(attr);
	minfo(INFO1, "Registered director pid: %u", user_director_pid);


	return 0;
}


/*
 * fix kernel 3.18.21 by Jan Friedl
 * family_ops are registred during family registration instead of genl_register_ops() in previous kernel version
 *
 *
 */

static struct genl_ops register_family_ops[] = {
    {
        .cmd = DIRECTOR_REGISTER_PID,
        .flags = 0,
        .policy = director_genl_policy,
        .doit = register_pid_handler,
        .dumpit = NULL,
    },
    {
        .cmd = DIRECTOR_SEND_GENERIC_USER_MESSAGE,
        .flags = 0,
        .policy = director_genl_policy,
        .doit = handle_send_generic_user_message,
        .dumpit = NULL,
    },
    {
        .cmd = DIRECTOR_NPM_RESPONSE,
        .flags = 0,
        .policy = director_genl_policy,
        .doit = generic_message_handler,
        .dumpit = NULL,
    },
    {
        .cmd = DIRECTOR_NODE_CONNECT_RESPONSE,
        .flags = 0,
        .policy = director_genl_policy,
        .doit = generic_message_handler,
        .dumpit = NULL,
    },
    {
        .cmd = DIRECTOR_IMMIGRATION_REQUEST_RESPONSE,
        .flags = 0,
        .policy = director_genl_policy,
        .doit = generic_message_handler,
        .dumpit = NULL,
    },
    {
        .cmd = DIRECTOR_ACK,
        .flags = 0,
        .policy = director_genl_policy,
        .doit = generic_message_handler,
        .dumpit = NULL,
    },
};


int init_director_comm(void) {
	int ret;

    mdbg(INFO4,"initializing director");
    minfo(INFO4,"initializing director");
    
    /* fix in kernel 3.18.21 by Jan Friedl */
	ret = genl_register_family_with_ops(&director_gnl_family, register_family_ops);
    if (ret != 0)
		return ret;

    mdbg(INFO4,"Director comm component initialized");
	minfo(INFO3, "Director comm component initialized");

	return 0;
}

void destroy_director_comm(void) {
	//int res;

//	if ( (res = shutdown_user_daemon_request()) ) {
//		printk(KERN_ERR "Failed to stop the helper daemon: %d\n", res);
//	}
	genl_unregister_family(&director_gnl_family);
}


/****************** Msg support ************************/

/** Helper method to create&send request */
static int msg_transaction_request(int msg_code, struct genl_tx* tx, struct msg_transaction_ops* ops, void* params, int interruptible) {
  int ret;
  void *msg_head;
  struct sk_buff *skb = NULL;
  int seq, director_pid;
  
  mdbg(INFO4,"msg_transaction_request");

  director_pid = is_director_connected();
  if ( !director_pid )
	return -EFAULT;

  skb = nlmsg_new(63000, GFP_KERNEL);
  if (skb == NULL)
      return -1;

  seq = get_unique_seq();

  msg_head = genlmsg_put(skb, director_pid, seq, &director_gnl_family, 0, msg_code);
  if (msg_head == NULL) {
      ret = -ENOMEM;
      goto failure;
  }
  
  ret = ops->create_request(skb, params);
  if (ret < 0)
      goto failure;

  ret = genlmsg_end(skb, msg_head);
  if (ret < 0)
      goto failure;

  ret = genlmsg_unicast_tx(skb, director_pid, tx, interruptible);
  if (ret != 0)
      goto failure;

  return ret;

failure:
  genlmsg_cancel(skb, msg_head);

  return ret;
}

static int msg_transaction_response(struct genl_tx* tx, struct msg_transaction_ops* ops, void* params) {
	struct sk_buff *skb = NULL;
	struct genl_info info;
	unsigned int ret = 0;
    mdbg(INFO4,"msg_transaction_response");

	if ( (ret= genlmsg_read_response(tx, &skb, &info, read_timeout)) )
		goto done;

	if ( (ret=check_for_error(&info)) )
		goto done;


	if ( ops->read_response ) {
	  ret = ops->read_response(&info, params);
	  if (ret < 0)
		  goto done;
	}

done:
	kfree_skb(skb);
	return ret;
}

int msg_transaction_do(int msg_code, struct msg_transaction_ops* ops, void* params, int interruptible) {
	struct genl_tx tx;

	int res = msg_transaction_request(msg_code, &tx, ops, params, interruptible);

	if ( res )
		return res;

	return msg_transaction_response(&tx, ops, params);
}
