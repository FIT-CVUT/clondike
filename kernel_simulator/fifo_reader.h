#ifndef FIFO_READER_H
#define FIFO_READER_H

#ifdef __cplusplus
extern "C" {
#endif

int create_fifo();

int destroy_fifo();

int open_fifo();

int close_fifo();

int try_read_fifo();

#ifdef __cplusplus
}
#endif

#endif
