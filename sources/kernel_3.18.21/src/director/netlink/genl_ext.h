#ifndef GENL_EXT_H
#define GENL_EXT_H

#include <net/genetlink.h>

struct sk_buff;

/* Helper functions that extend generic netlink functionality to better support kernel-as-client functionality */

/* Structure representing one request-response transaction */
struct genl_tx {
	u8 cmd;  /*Command type */
	u32 seq; /* Sequence number */
};

/* not used now */
/* Multicast group for Netlink family */
/* static const struct genl_multicast_group gnl_mcgrps[] = {{.name = "clondike",},}; */

/**
  * genlmsg_unicast_tx - wrapper around generic netlink unicast. 
  * It addition to a standard version it has an option to register transaction for reading the response
  * 
  * @param skb: netlink message as socket buffer
  * @param pid: netlink pid of the destination socket
  * @param tx: the transaction context that can be later used for reading the response. If NULL then only standard unicast is performed
  * @param interuptible: If !=0, the transaction can be terminated when signal arrives and does not need to wait for a reply
  */
int genlmsg_unicast_tx(struct sk_buff *skb, u32 pid, struct genl_tx* tx, int interuptible);

/**
  * Blocking read call that will read netlink response for the specified transaction
  *
  * @param tx: Transaction previously used in genlms_unicast_tx
  * @param skb: will hold resulting buffer (and reference to it, that must be freed by the caller)
  * @param info: will hold resulting genl_info.. it should be a pointer to already allocated structure, and its data will be just filled
  * @param timeout: timeout in seconds
  */
int genlmsg_read_response(struct genl_tx* tx, struct sk_buff **skb, struct genl_info *info, int timeout);



/**
  * This handler will distribute the message to registered transactions 
  *
  * @param skb: netlink message as socket buffer
  * @param info: receiving information
  * */
int generic_message_handler(struct sk_buff *skb, struct genl_info *info);

#endif
