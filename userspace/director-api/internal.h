#ifndef INTERNAL_H
#define INTERNAL_H

/** Structure keeping track of current state of netlink connection */
struct internal_state {
	/* Family id of the generic netlink channel for director */
	uint16_t gnl_fid;
	/* Socket for the netlink connection */
	struct nl_sock *sk;	
};

/** Reads current state */
extern struct internal_state* get_current_state(void);

#endif
