#ifndef _CTLFS_H
#define _CTLFS_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

int init_ctlfs();

int destroy_ctlfs();

int ctlfs_init_dirs(void);

int ctlfs_stop_dirs(void);

int ctlfs_init_files(void);

int ctlfs_stop_files(void);

int create_pen_node_directory(struct sockaddr_in * node_addr, int index);

int create_ccn_node_directory(struct sockaddr_in * node_addr, int index);

void inc_ccn_count();

void dec_ccn_count();

void inc_pen_count();

void dec_pen_count();

#ifdef __cplusplus
}
#endif


#endif


