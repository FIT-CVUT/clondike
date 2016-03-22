#include "kkc.h"
#include "kkc_socket_manager.h"
#include "process_manager.h"
#include "clondike_kernel_simulator.h"
#include "netlink_message.h"
#include "ctlfs.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctype.h>
#include <sstream>

#define MAX_INCOMMING_CONNECTIONS 100
#define BUFSIZE 1000
using namespace std;



static int main_socket;

extern vector<struct kkc_socket * > kkc_sockets;

int start_ccn(){
    if ((main_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        cout << "cannot create main socket!" << endl;
        return -1;
    }

    struct sockaddr_in main_sock_address;
    if (get_address_from_file("/clondike/ccn/listen", &main_sock_address, 0) == -1){
        cout << "cannot get address from file" << endl;
        return -1;
    }
    
    if (bind(main_socket, (sockaddr *) &main_sock_address, sizeof(main_sock_address)) == -1){
        cout << "cannot assign address to socket" << endl;
        return -1;
    }

    if (listen(main_socket, MAX_INCOMMING_CONNECTIONS) == -1){
        cout << "Cannot liste on the main address" << endl;
        return -1;
    }
    return 0;
}

void try_receive_ccn(){
    fd_set socket_set;
    timeval tv;
    int max_socket = main_socket;
    FD_ZERO(&socket_set);
    FD_SET(main_socket, &socket_set);
    
    for(std::vector<struct kkc_socket * >::iterator it = kkc_sockets.begin(); it != kkc_sockets.end(); it++){
        cout << "index:" << (*it)->index << " recv:" << (*it)->receiving_socket << " send:" << (*it)->sending_socket << endl;
        if((*it)->receiving_socket != 0){
            FD_SET((*it)->receiving_socket, &socket_set);
            max_socket = max(max_socket, (*it)->receiving_socket);
        }
    }
    cout << "kkc_sockets size: " << kkc_sockets.size() << endl;
    //wait max 5ms
    tv.tv_sec = 0;
    tv.tv_usec = 5;
    int ret;
    if ( (ret = select(max_socket + 1, &socket_set, NULL, NULL, &tv)) > 0){
        cout << "after select" << endl; 
        if(FD_ISSET(main_socket, &socket_set)){
            struct sockaddr_in pen_node_addr;
            socklen_t addrlen = sizeof(pen_node_addr);
            int pen_node = accept(main_socket, (sockaddr*)&pen_node_addr, &addrlen);
        
            int ret = kkc_socket_push_receiving(pen_node, &pen_node_addr);
            if(ret == -1){
                printf("receiving socket already exists\n");
            }
            else{
                cout << "receiving socket connected: " << inet_ntoa(pen_node_addr.sin_addr) << " socket: " << pen_node << endl;
                 netlink_send_node_connected(&pen_node_addr, ret);
                 create_pen_node_directory(&pen_node_addr, ret);
                 inc_pen_count();
            }

        }

        for(std::vector<struct kkc_socket * >::iterator it = kkc_sockets.begin(); it != kkc_sockets.end(); it++){
            if((*it)->receiving_socket != 0 && FD_ISSET((*it)->receiving_socket, &socket_set)){
                cout << "ready for receivei: " << (*it)->receiving_socket << endl;
                struct kkc_message *msg;
                if (kkc_receive_message((*it)->receiving_socket, &msg) < 0){
                    //socket no longer available, connection closed
                    cout << "socket " << (*it)->receiving_socket << " closed" << endl;
                    (*it)->receiving_socket = 0;
                    close((*it)->sending_socket);
                    (*it)->sending_socket = 0;
                    netlink_send_node_disconnected((*it)->index, 0, 0);
                    dec_pen_count();
                    dec_ccn_count();

                    continue;
                }
                kkc_handle_message(msg, (*it)->index);
            }
        }
    }
    //cout << "ret: " << ret << endl;
}

void kkc_close_connections(){
    for(std::vector<struct kkc_socket *>::iterator it = kkc_sockets.begin(); it != kkc_sockets.end(); it++){
        if ((*it)->receiving_socket)
            close((*it)->receiving_socket);

        if((*it)->sending_socket)
            close((*it)->sending_socket);
    }
    
    close(main_socket);
}



int get_address_from_file(const char * filename, struct sockaddr_in * server_address, int clear){

    string line;
    ifstream fin(filename);

    if (!fin.is_open()){
        cout << "cannot open file " << filename << " for reading address." << endl;
    }

    fin >> line;
    fin.close();

    if (clear){
        ofstream fout(filename, ios::trunc| ios::out);
        fout.close();
    }

    size_t addr_begin = line.find(':');
    size_t addr_end = line.find(':', addr_begin + 1);
    cout << line << endl;
    if (line.size() == 0 || addr_begin == addr_end)
        return 1;

    string addr = line.substr(addr_begin + 1, addr_end - addr_begin - 1);
    
    int port = atoi(line.substr(addr_end + 1).c_str());
    cout << "address: " << addr << " port: " << port << endl;

    server_address->sin_family = AF_INET;
    server_address->sin_port = htons(port);
    inet_aton(addr.c_str(), &(server_address->sin_addr));

    return 0;
}

int ccn_connect(){
    int s;
    if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        cout << "cannot create socket!" << endl;
        return -1;
    }

    struct sockaddr_in s_address;
    get_address_from_file("/clondike/pen/connect", &s_address, 1);

    if (kkc_pen_already_connected(&s_address) > 0){
        cout << "already connected" << endl;
        return 0;
    }

    if (connect(s, (struct sockaddr *) &s_address, sizeof(s_address)) < 0){
        cout << "cannot connect to the peer"  << endl;
        return -1;
    }

    kkc_socket_push_sending(s, &s_address);
    cout << "successfuly connected to socket: " << s << endl;

    inc_ccn_count();

    return s;
}

int kkc_receive_message(int fd, struct kkc_message ** ret_msg){
    int total_received = 0;
    int len;
    
    if (kkc_sockets.size() == 0){
        *ret_msg = NULL;
        return 0;
    }

    struct kkc_message * msg = (struct kkc_message *) malloc(sizeof(struct kkc_message)); 

    //receive message header
    
    cout << "receiving message" << endl;
    while(total_received < 4){
        len = recv(fd, &msg[total_received], sizeof(kkc_message_header) - total_received, 0);
        if (len <= 0)
            return -1;
        total_received += len;
        cout << "header - received bytes: " << len << endl;
    }

    //receive body

    char * data = (char *) malloc (sizeof(char) * (msg->hdr.len - sizeof(struct kkc_message_header)));

    int total_data_received = 0;
    while(total_received < msg->hdr.len){
        len = recv(fd, &data[total_data_received], 
            msg->hdr.len - sizeof(struct kkc_message_header) - total_data_received, 0);
        
        if (len <= 0){
            free(msg);
            free(data);
            return -1;
        }

        total_data_received += len;
        total_received += len;
        cout << "body - received bytes: " << len << endl;
    }
    
    memcpy(msg->data, data, total_data_received);
   
    free(data);
    *ret_msg = msg;
    
    return 0;
}

int kkc_free_msg(struct kkc_message *msg){
    free(msg->data);
    free(msg);

    return 0;
}

struct kkc_attr * kkc_create_attr_u32(int type, uint32_t value){
    struct kkc_attr * attr = (struct kkc_attr *) malloc(sizeof(kkc_attr));
    
    attr->hdr.len = sizeof(uint32_t) + sizeof(kkc_attr_header);
    attr->hdr.type = type;
    memcpy(attr->data, (char *)&value, sizeof(uint32_t));

    return attr;
}

struct kkc_attr * kkc_create_attr_string(int type, const char * value){
    struct kkc_attr * attr = (struct kkc_attr *) malloc(sizeof(kkc_attr));

    memcpy(attr->data, value, sizeof(char) * strlen(value) + 1);
    attr->hdr.len = sizeof(char) * strlen(value) + 1 + sizeof(kkc_attr_header);
    attr->hdr.type = type;

    return attr;
}


int kkc_send_all(int fd, const char * buf, int buf_len){
    int total = 0;
    int len;


    while(total < buf_len){
        len = send(fd, buf + total, buf_len - total, 0);
        if (len == -1)
            return -1;

        total += len;
    }

    cout << "send " << total << "bytes" << endl;

    return total;
}

void kkc_dump_msg(const char * ptr, int buflen){
  unsigned char *buf = (unsigned char*)ptr;
  int i, j;
  for (i=0; i<buflen; i+=16) {
    printf("%06x: ", i);
    for (j=0; j<16; j++) 
      if (i+j < buflen){
        printf("%02x ", buf[i+j]);
      }
      else
        printf("   ");
    printf(" ");
    for (j=0; j<16; j++) 
      if (i+j < buflen)
        printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
    printf("\n");
  }
}

void kkc_handle_message(struct kkc_message * msg, int peer_index){
    cout << "handle msg" << endl;
    kkc_dump_msg((char *)msg, msg->hdr.len);
    switch(msg->hdr.type){
        case EMIG_REQUEST:
            handle_emig_request_message(msg, peer_index);
            break;
        case EMIG_REQUEST_RESPONSE:
            handle_emig_request_response_message(msg, peer_index);
            break;
        case EMIG_BEGIN:
            handle_emig_begin_message(msg, peer_index);
            break;
        case EMIG_DONE:
            handle_emig_done_message(msg, peer_index);
            break;
    }
}

int get_socket(int index){
    for(std::vector<struct kkc_socket * >::iterator it = kkc_sockets.begin(); it != kkc_sockets.end(); it++){
        if((*it)->index == index)
            return (*it)->sending_socket;
    }

    return -1;
}

void handle_emig_request_message(struct kkc_message *msg, int peer_index){
    cout << "handle emig request" << endl;
    struct kkc_attr_header attr;
    int pid;
    int uid;
    char name[MAX_DATA_LEN];

    char * buf = (char *) msg;

    //get pid header
    buf += sizeof(struct kkc_message_header);

    memcpy(&attr, buf, sizeof(struct kkc_attr_header));
    buf += sizeof(struct kkc_attr_header);
    memcpy(&pid, buf, attr.len - sizeof(struct kkc_attr_header));
   
    //get uid header
    buf += attr.len - sizeof(struct kkc_attr_header);
    memcpy(&attr, buf, sizeof(struct kkc_attr_header));
    buf += sizeof(struct kkc_attr_header);
    memcpy(&uid, buf, attr.len - sizeof(struct kkc_attr_header));

    //get name header
    buf += attr.len - sizeof(struct kkc_attr_header);
    memcpy(&attr, buf, sizeof(struct kkc_attr_header));

    buf += sizeof(struct kkc_attr_header);
    memcpy(name, buf, attr.len - sizeof(struct kkc_attr_header));

    cout << "pid: " << pid << endl;
    cout << "uid: " << uid << endl;
    cout << "name: " << name << endl;
   
    imig_process_put(pid, name, uid, peer_index);
}
void handle_emig_request_response_message(struct kkc_message * msg, int peer_index){
    cout << "handle emig request response" << endl;
    struct kkc_attr_header attr;
    int pid;
    int decision;

    char * buf = (char *) msg;

    //get pid header
    buf += sizeof(struct kkc_message_header);

    memcpy(&attr, buf, sizeof(struct kkc_attr_header));
    buf += sizeof(struct kkc_attr_header);
    memcpy(&pid, buf, attr.len - sizeof(struct kkc_attr_header));
    buf += attr.len - sizeof(struct kkc_attr_header);
   
    //get uid header
    memcpy(&attr, buf, sizeof(struct kkc_attr_header));
    buf += sizeof(struct kkc_attr_header);
    memcpy(&decision, buf, attr.len - sizeof(struct kkc_attr_header));

    cout << "pid: " << pid << endl;
    cout << "decision: " << decision << endl;

    //it is possible, that decision == DO_NOT_MIGRATE, we will still call this to erase process from pid manager
    emig_process_migration_confirmed(pid, decision);
}

void handle_emig_begin_message(struct kkc_message * msg, int peer_index){
    cout << "handle emig begin" << endl;
    
    struct kkc_attr_header attr;
    int pid;
    int uid;
    char name[MAX_DATA_LEN];

    char * buf = (char *) msg;

    //get pid header
    buf += sizeof(struct kkc_message_header);

    memcpy(&attr, buf, sizeof(struct kkc_attr_header));
    buf += sizeof(struct kkc_attr_header);
    memcpy(&pid, buf, attr.len - sizeof(struct kkc_attr_header));
   
    //get uid header
    buf += attr.len - sizeof(struct kkc_attr_header);
    memcpy(&attr, buf, sizeof(struct kkc_attr_header));
    buf += sizeof(struct kkc_attr_header);
    memcpy(&uid, buf, attr.len - sizeof(struct kkc_attr_header));

    //get name header
    buf += attr.len - sizeof(struct kkc_attr_header);
    memcpy(&attr, buf, sizeof(struct kkc_attr_header));

    buf += sizeof(struct kkc_attr_header);
    memcpy(name, buf, attr.len - sizeof(struct kkc_attr_header));

    cout << "pid: " << pid << endl;
    cout << "uid: " << uid << endl;
    cout << "name: " << name << endl;
  
    if (imig_process_start_migrated_process(pid) < 0){
        cout << "cannot start migrated process, no such PID" << endl;
    }
}

void handle_emig_done_message(struct kkc_message * msg, int peer_index){
    cout << "handle emig done" << endl;

    struct kkc_attr_header attr;
    int pid;
    int return_code;

    char * buf = (char *) msg;

    //get pid header
    buf += sizeof(struct kkc_message_header);

    memcpy(&attr, buf, sizeof(struct kkc_attr_header));
    buf += sizeof(struct kkc_attr_header);
    memcpy(&pid, buf, attr.len - sizeof(struct kkc_attr_header));
    buf += attr.len - sizeof(struct kkc_attr_header);
   
    //get uid header
    memcpy(&attr, buf, sizeof(struct kkc_attr_header));
    buf += sizeof(struct kkc_attr_header);
    memcpy(&return_code, buf, attr.len - sizeof(struct kkc_attr_header));

    cout << "pid: " << pid << endl;
    cout << "return_code: " << return_code << endl;

    emig_process_done(pid, return_code);
}

