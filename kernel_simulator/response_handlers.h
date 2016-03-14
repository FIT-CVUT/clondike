#ifndef RESPONSE_HANDLERS_H
#define RESPONSE_HANDLERS_H

#include <netlink/netlink.h>
#include <netlink/genl/mngt.h>

int ack_handler(struct nl_cache_ops * c, struct genl_cmd * cmd, struct genl_info *info, void *arg);

int director_npm_response_handler(struct nl_cache_ops * c, struct genl_cmd * cmd, struct genl_info *info, void *arg);

int director_node_connect_response(struct nl_cache_ops * c, struct genl_cmd * cmd, struct genl_info *info, void *arg);

int director_immigration_request_response(struct nl_cache_ops * c, struct genl_cmd * cmd, struct genl_info *info, void *arg);


#endif
