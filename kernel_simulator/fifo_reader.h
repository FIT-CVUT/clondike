#ifndef FIFO_READER_H
#define FIFO_READER_H

#ifdef __cplusplus
extern "C" {
#endif

int init_process_reader();

int try_read_processes();

int send_process_exit(int fd, int return_code);

#ifdef __cplusplus
}
#endif

#endif
