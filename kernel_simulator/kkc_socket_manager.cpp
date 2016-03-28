#include "kkc_socket_manager.h"

#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>

using namespace std;

vector<kkc_socket *> kkc_sockets;
static int max_index = 0;


int kkc_socket_push_receiving(int socket, struct sockaddr_in * recv_addr){
    printf("pushing to sockets receiving\n");
    for(vector<kkc_socket *>::iterator it = kkc_sockets.begin(); it != kkc_sockets.end(); it++){
        if ((*it)->s_addr == recv_addr->sin_addr.s_addr){
            if((*it)->receiving_socket == 0){
                (*it)->receiving_socket = socket;
                return (*it)->index;
            }
            else{
                printf("ERROR: already connected\n");
                return -1; //already connected
            }
        }
    }

    struct kkc_socket * s;
    s = (struct kkc_socket *) malloc(sizeof(struct kkc_socket));

    s->index = max_index++;
    s->sending_socket = 0;
    s->receiving_socket = socket;
    s->s_addr = recv_addr->sin_addr.s_addr;

    kkc_sockets.push_back(s);

    return s->index;
}

int kkc_socket_push_sending(int socket, struct sockaddr_in * send_addr){
    printf("pushing to sockets sending\n");

    printf("address: %s\n", inet_ntoa(send_addr->sin_addr));
    printf("port: %d\n", ntohs(send_addr->sin_port));


    for(vector<kkc_socket *>::iterator it = kkc_sockets.begin(); it != kkc_sockets.end(); it++){
        if ((*it)->s_addr == send_addr->sin_addr.s_addr){
            if((*it)->sending_socket == 0){
                (*it)->sending_socket = socket;
                return (*it)->index;
            }
            else{
                printf("ERROR: already connected\n");
                return -1; //already connected
            }
        }

    }
    
    struct kkc_socket * s;
    s = (struct kkc_socket *) malloc(sizeof(struct kkc_socket));

    s->index = max_index++;
    s->sending_socket = socket;
    s->receiving_socket = 0;
    s->s_addr = send_addr->sin_addr.s_addr;

    kkc_sockets.push_back(s);

    return s->index;
}




int kkc_pen_already_connected(struct sockaddr_in * s_address){
    for(std::vector<struct kkc_socket * >::iterator it = kkc_sockets.begin(); it != kkc_sockets.end(); it++){
        if((*it)->s_addr == s_address->sin_addr.s_addr && (*it)->sending_socket != 0 ){
            return 1;
        }
    }
    return 0;
}

int kkc_erase_disconnected_sockets(){
    int clean = 0;
    while(!clean){
        clean  = 1;
        for(std::vector<struct kkc_socket * >::iterator it = kkc_sockets.begin(); it != kkc_sockets.end(); it++){
            if((*it)->receiving_socket == 0 && (*it)->sending_socket == 0 ){
                free(*it);
                kkc_sockets.erase(it);
                clean = 0;
                break;
            }
        }
    }

    return 0;
}

