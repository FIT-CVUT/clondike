#ifndef KKC_SOCKET_MANAGER_H
#define KKC_SOCKET_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

struct kkc_socket{
    int index;
    int receiving_socket;
    int sending_socket;
    unsigned long s_addr;
};



int kkc_socket_push_receiving(int socket, struct sockaddr_in * recv_addr);

int kkc_socket_push_sending(int socket, struct sockaddr_in * send_addr);

int kkc_pen_already_connected(struct sockaddr_in * s_address);

int kkc_erase_disconnected_sockets();

#ifdef __cplusplus
}
#endif

#endif
