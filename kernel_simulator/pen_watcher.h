#ifndef PEN_WATCHER_H
#define PEN_WATCHER_H

//8 is strlen("connect") file name + \0
#define BUF_LEN  (10 * (sizeof(struct inotify_event) + 8))

void init_pen_watcher();

void close_pen_watcher();

int check_pen_watcher();


#endif
