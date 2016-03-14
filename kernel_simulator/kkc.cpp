#include "kkc.h"

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


#define MAX_INCOMMING_CONNECTIONS 100
#define BUFSIZE 1000
using namespace std;



static int main_socket;
static vector<int> sockets;



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
    
    for(std::vector<int>::iterator it = sockets.begin(); it != sockets.end(); it++){
        FD_SET(*it, &socket_set);
        max_socket = max(max_socket, *it);
    }
    cout << "main socket: " << main_socket << endl;
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
            sockets.push_back(pen_node);
            cout << "connected to client: " << inet_ntoa(pen_node_addr.sin_addr) << " socket: " << pen_node << endl;
        }

        for(std::vector<int>::iterator it = sockets.begin(); it != sockets.end(); it++){
            if(FD_ISSET(*it, &socket_set)){
                char buf[BUFSIZE];
                int length;
                if ( (length=recv(*it, buf, BUFSIZE - 1, 0)) <= 0 ){
                    //zero or less bytes when receive means error
                    close(*it);
                    sockets.erase(it);
                    //if we erase some socket, we will leave iteration
                    break;
                }
                else{
                    //handle message
                    cout << buf << endl;
                }
            }
        }
    }
    cout << "ret: " << ret << endl;
}

void close_connections(){
    close(main_socket);
    for(std::vector<int>::iterator it = sockets.begin(); it != sockets.end(); it++){
        close(*it);
    }
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

void ccn_send(unsigned int index, const char * msg){
    if (index < sockets.size())
        send(sockets[index], msg, strlen(msg), 0);
}

void ccn_connect(){
    int s;
    if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        cout << "cannot create socket!" << endl;
    }

    struct sockaddr_in s_address;
    get_address_from_file("/clondike/pen/connect", &s_address, 1);

    if (connect(s, (struct sockaddr *) &s_address, sizeof(s_address)) < 0){
        cout << "cannot connect to the peer"  << endl;
    }
}

/*
int main() {
    
    struct sockaddr_in addr;
    cout << get_address_from_file("/clondike/pen/connect", &addr, 1) << endl;
}

*/


