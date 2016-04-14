#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdint.h>

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

    printf("sizeof NULL: %d\n", sizeof(*NULL));

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
    printf("sizeof argv[2]: %d\n", len);
    char send_buf[516];
    memcpy(send_buf, &len, sizeof(len));
    memcpy(send_buf + sizeof(len), argv[2], len);

    send_all(sock, send_buf, len + sizeof(len));

printf("read\n");
    read(sock, buf, 1);
    printf("%c", buf[0]);
    close(sock);

    return 0;
}
