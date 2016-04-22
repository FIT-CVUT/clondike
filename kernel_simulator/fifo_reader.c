#include "netlink_message.h"
#include "process_manager.h"
#include "kkc_messages.h"
#include "kkc.h"
#include "pid_manager.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/select.h>

#define MAX_UNIX_SOCKET_CONNECTIONS	50000
#define UNIX_SOCKET_PATH "/var/run/clondike.sock"

static int process_endpoint_fd;

static uint64_t get_jiffies(){
    float uptime;
    FILE* proc_uptime_file = fopen("/proc/uptime", "r");
    if (proc_uptime_file == NULL){
        return 0;
    }
    fscanf(proc_uptime_file, "%f", &uptime);
    
    fclose(proc_uptime_file);

    uint64_t uptime_ms = uptime * 1000.0;
    return uptime_ms;
}


int init_process_reader(){
    unlink(UNIX_SOCKET_PATH);
    if ((process_endpoint_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        printf("cannot create process endpoint socket\n");
        return -1;
    }
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, UNIX_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if( bind(process_endpoint_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1){
        printf("cannot bind process endpoint socket\n");
        return -1;
    }

    if( listen(process_endpoint_fd, MAX_UNIX_SOCKET_CONNECTIONS) == -1){
        printf("cannot listen process endpoint socket\n");
        return -1;
    }
    printf("process reader initialized\n");
    return 0;
}

static int read_all(int fd, char * buf, int buflen){
    int total_received = 0;
    int len;

    uint32_t header;

//receive message header

    while(total_received < 4){
        len = recv(fd, &buf[total_received], 4 - total_received, 0);
        if (len <= 0)
            return -1;
        total_received += len;
    }

    memcpy(&header, buf, 4);


    int total_data_received = 0;
    while(total_received < header){
        len = recv(fd, &buf[total_data_received], header - total_data_received, 0);

        if (len <= 0){
            return -1;
        }

        total_data_received += len;
        total_received += len;
    }

    return total_data_received;
} 

int try_read_processes(){
    fd_set socket_set;
    struct timeval tv;
    FD_ZERO(&socket_set);
    FD_SET(process_endpoint_fd, &socket_set);
    char buf[BUF_SIZE];
    char buf_line[BUF_SIZE];
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    int ret = 0;

    if ( (ret = select(process_endpoint_fd + 1, &socket_set, NULL, NULL, &tv)) > 0){
        if (FD_ISSET(process_endpoint_fd, &socket_set)){
            int new_fd;
            if ( (new_fd = accept(process_endpoint_fd, NULL, NULL)) == -1){
                printf("accept connection failed\n");
                return -1;
            }
            bzero(buf, sizeof(BUF_SIZE));
            int bytes = read_all(new_fd, buf, BUF_SIZE);
            if (bytes == 0){
                printf("connection ends\n");
                return 0;
            } else if (bytes <= -1){
                printf("read error");
                return -1;
            }
            printf("buffer: \"%s\"\n", buf);

            memcpy(buf_line, buf, bytes);
            buf_line[bytes] = '\0';

            char * argv[50];
            const char * const envp[] = {"envp", "EMIG=1", NULL};
            char * name = strtok(buf, " ");
            if (strlen(name) == 0){
                send_process_exit(new_fd, 0);
                return 0;
            }
            char * arg = NULL;
            int arg_num = 0;
            for(arg_num = 0; arg_num < 50; arg_num++){
                arg = strtok(NULL, " ");
                argv[arg_num] = arg;
                if(arg == NULL)
                    break;
            }
	    int uid = 0; //root user
	    int pid = get_next_pid();
	    uint64_t jiffies = get_jiffies();
            netlink_send_npm_check_full(pid, uid, 0, name, jiffies, (const char * const * )argv, envp);
            emig_process_put(pid, name, 0, get_sequence_number(), jiffies, new_fd, buf_line);
        }
    }

    return 0;
}

int send_process_exit(int fd, int return_code){
    char s[10];
    sprintf(s, "%d", return_code);
    write(fd, s, 1);
    close(fd);
    return 0;
}


/*
int main(){

    init_process_reader();   
    int i;
    for(i = 0; i < 10; i++){
        try_read_processes();
        sleep(1);
    }

    unlink(UNIX_SOCKET_PATH);
}

*/


