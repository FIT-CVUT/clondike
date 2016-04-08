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

#define FIFO_PATH "/var/run/clondike.pipe"

static int fifo_fd;
static FILE * fifo_file;

static struct stat st = {0};

int create_fifo(){
    if(stat(FIFO_PATH, &st) == -1){
        if (mknod(FIFO_PATH, S_IFIFO|0666, 0) != 0){
            printf("cannot create clondike FIFO\n");
            return -1;
        }
    }
    return 0;
}

int destroy_fifo(){
    if (stat(FIFO_PATH, &st) == 0){
        if(unlink(FIFO_PATH) != 0){
            printf("cannot delete clondike FIFO\n");
            return 1;
        }   
    }
    return 0;
}

int open_fifo(){
    fifo_fd = open(FIFO_PATH, O_RDWR | O_NONBLOCK);
    if (fifo_fd == -1){
        printf("cannot open clondike FIFO\n");
        exit(1);
    }

    //for reading
    fifo_file = fdopen(fifo_fd, "r");
    if (fifo_file == NULL){
        printf("cannot open clondike FIFO file\n");
    }

    return 0;
}

int close_fifo(){
    if(fifo_file)
        fclose(fifo_file);

    if(fifo_fd != -1)
        close(fifo_fd);
}

static uint64_t get_jiffies(){
    float uptime;
    FILE* proc_uptime_file = fopen("/proc/uptime", "r");
    fscanf(proc_uptime_file, "%f", &uptime);

    uint64_t uptime_ms = uptime * 1000.0;
    return uptime_ms;
}

int try_read_fifo(){
    fd_set fifo_set;

    FD_ZERO(&fifo_set);
    FD_SET(fifo_fd, &fifo_set);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    char *line = NULL;
    int len = 0;
    size_t alloc_size = 0;

    if (select(fifo_fd + 1, &fifo_set, NULL, NULL, &tv) > 0){
        len = getline(&line, &alloc_size, fifo_file);
        line[len-1] = '\0';
        printf("fifo: %s\n", line); //already contains newline
        const char * const argv[] = {"argv", NULL};
        const char * const envp[] = {"envp", "EMIG=1", NULL};
	int uid = 0; //root user
	int pid = get_next_pid();
	uint64_t jiffies = get_jiffies();
        netlink_send_npm_check_full(pid, uid, 0, line, jiffies, argv, envp);
        emig_process_put(pid, line, 0, get_sequence_number(), jiffies);
        free(line);
    }
}



/*
int main(){

    create_fifo();
    
    open_fifo();

    int i;
    for(i = 0; i < 10; i++){
        try_read_fifo();
        sleep(1);
    }

    close_fifo();
    destroy_fifo();
}

*/
