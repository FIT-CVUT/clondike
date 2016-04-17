#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define BUF_SIZE 2048

int send_all(int fd, const char * buf, int buf_len){
    int total = 0;
    int len;


    while(total < buf_len){
        len = send(fd, buf + total, buf_len - total, 0);
        if (len == -1)
            return -1;

        total += len;
    }


    return total;
}

int main(int argc, char * argv[]){
    int sock;
    struct sockaddr_un server;
    char buf[2];

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("cannot open socket\n");
        return -1;
    }

    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, argv[1]);

    if (connect(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0){
        close(sock);
        perror("cannoct connect to the unix domain socket\n");
        return -1;
    }

    uint32_t len = strlen(argv[2]);
    len = len < BUF_SIZE? len : BUF_SIZE;
    
    char send_buf[BUF_SIZE];
    memcpy(send_buf, &len, sizeof(len));
    memcpy(send_buf + sizeof(len), argv[2], len);
    send_buf[sizeof(len) + len - 1] = '\0';

    send_all(sock, send_buf, len + sizeof(len));

    read(sock, buf, 1);
    close(sock);

    return 0;
}
