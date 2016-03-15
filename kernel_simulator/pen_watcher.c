#include "pen_watcher.h"

#include <sys/inotify.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>

static int inotify_fd;


void init_pen_watcher(){
    int inotify_fd;

    if ( (inotify_fd = inotify_init()) == -1){
        printf("cannot init inotify\n");
    }

    printf("inotify fd=%d\n", inotify_fd);

    if ( inotify_add_watch(inotify_fd, "/clondike/pen/connect", IN_MODIFY) == -1){
        printf("cannot watch /clondike/pen/connect connection file\n");
    }
    else{
        printf("Watching /clondike/pen/connect\n");
    }
}

void close_pen_watcher(){
    close(inotify_fd);
}


int check_pen_watcher(){
    int length; 
    char buffer [BUF_LEN];
    
    printf("check pen watcher\n");

    fd_set socket_set;
    struct timeval tv;

    FD_ZERO(&socket_set);
    FD_SET(inotify_fd, &socket_set);

    tv.tv_sec = 0;
    tv.tv_usec = 5;

    int i;
    int ret = 0;

    if(select(inotify_fd + 1, &socket_set, NULL, NULL, &tv) > 0){
        if(FD_ISSET(inotify_fd, &socket_set)){
            length = read(inotify_fd, buffer, BUF_LEN);
            printf("watcher length: %d\n", length);
            if (length == 0)
                return 0; 
        
            for (i = 0; i < length; i++){
                printf("checking events\n");
                struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];     
                if ( event->mask & IN_MODIFY ){
                    printf("pen modified\n");
                    ret = 1;
                }

            }
        }
    }

    return ret;
}
