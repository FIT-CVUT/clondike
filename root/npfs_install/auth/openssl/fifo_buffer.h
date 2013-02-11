#ifndef FIFO_BUFFER_H
#define FIFO_BUFFER_H

#include <pthread.h>
#include <stdio.h>

#define MAX_FIFO_BUFFER_CAPACITY 60000

/* Circular fifo buffer with max capacity == MAX_FIFO_BUFFER_CAPACITY */
struct fifo_buffer {
	char buffer[MAX_FIFO_BUFFER_CAPACITY];
	int pos; /* Current position in the buffer */
	int size; /* Amount of data in the buffer */
	pthread_mutex_t lock;
	pthread_cond_t cond;
};

static inline void init_buffer(struct fifo_buffer* fbuf) {	
	int res;

	printf("Init buffer\n");

	fbuf->pos = 0;
	fbuf->size = 0;
	
	res = pthread_mutex_init(&fbuf->lock, NULL);
	if ( res ) {
		printf("RES: %d\n", res);
	}

	res = pthread_cond_init(&fbuf->cond, NULL);
	if ( res ) {
		printf("RES: %d\n", res);
	}
};

static inline void destroy_buffer(struct fifo_buffer* fbuf) {
	printf("Destroy buffer\n");

	pthread_mutex_destroy(&fbuf->lock);
	pthread_cond_destroy(&fbuf->cond);
};

/* If the data cannot fit into the buffer, return -1, otherwise count of written bytes */
int write_to_buffer(struct fifo_buffer* fbuf, const char* data, int data_length);
/* 
  Reads requested_bytes count from buffer and returns bytes read. If there is not enough bytes in the buffer, block 
  Resulting buffer is supposed to be already allocated
*/
int read_from_buffer(struct fifo_buffer* fbuf, int requested_bytes, char* result, int* result_length);

#endif
