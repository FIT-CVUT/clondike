#ifndef KKC_H
#define KKC_H
#ifdef __cplusplus
extern "C" {
#endif

int start_ccn();

void try_receive_ccn();

void close_connections();

int get_address_from_file(const char * filename, struct sockaddr_in * server_address,  int clear);

void ccn_connect();

void ccn_send(unsigned int index, const char * msg);
#ifdef __cplusplus
}
#endif

#endif
