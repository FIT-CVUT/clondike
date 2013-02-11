#include "fifo_buffer.h"
#include <errno.h>
#include <string.h>
#include <time.h>

#define min_int(a,b) ((a)<(b)) ? (a) : (b)

int write_to_buffer(struct fifo_buffer* fbuf, const char* data, int data_length) {
	int res = 0;
	int from;
	int first_part_size;

	pthread_mutex_lock(&fbuf->lock);
	if ( fbuf->size + data_length > MAX_FIFO_BUFFER_CAPACITY ) {
		res = -1;
		goto done;
	}

	from = (fbuf->pos + fbuf->size) % MAX_FIFO_BUFFER_CAPACITY;
	first_part_size = min_int(MAX_FIFO_BUFFER_CAPACITY - from, data_length);
//	printf("WRITE COPY: %d - %d\n" , from, first_part_size);
	memcpy(fbuf->buffer + from, data, first_part_size);

	if ( first_part_size < data_length ) {
		/* We have overflow over the buffer end => handle the rest */
		memcpy(fbuf->buffer, data + first_part_size, data_length - first_part_size);
//		printf("WRITE COPY: %d - %d\n" , 0, data_length - first_part_size);
	}	

//	printf("FIFO writing.. pos: %d size: %d len: %d\n", fbuf->pos, fbuf->size, data_length);	
	//fbuf->pos = ( fbuf->pos + data_length ) % MAX_FIFO_BUFFER_CAPACITY;
	fbuf->size += data_length;
//	printf("FIFO post write. pos: %d size: %d\n", fbuf->pos, fbuf->size);	
	pthread_cond_broadcast(&fbuf->cond);
//	printf("BROADCASTED\n");
	fflush(stdout);

	res = data_length;
done:
	pthread_mutex_unlock(&fbuf->lock);
	return res;
}

int read_from_buffer(struct fifo_buffer* fbuf, int requested_bytes, char* result, int* result_length) {	
	int res = 0;
	int first_part_size = 0;
	struct timespec timeout;

	clock_gettime(CLOCK_REALTIME, &timeout);	
	timeout.tv_sec += 15;

//	printf("FIFO read request..%d\n", requested_bytes);

	if ( requested_bytes > MAX_FIFO_BUFFER_CAPACITY ) {
		printf("Invalid request for %d bytes\n", requested_bytes);
		return -EINVAL;
	}

	res = pthread_mutex_lock(&fbuf->lock);
		if ( res ) {
//			printf("LOCK RES: %d\n", res);
			goto done;
		}

	
	while ( requested_bytes > fbuf->size ) {
//		printf("FIFO Waiting: requested %d size %d\n", requested_bytes, fbuf->size);
		res = pthread_cond_timedwait(&fbuf->cond, &fbuf->lock, &timeout);
		if ( res ) {
			res = -res;
//			printf("RES: %d\n", res);
			goto done;
		}
	}

//	printf("FIFO reading.. pos: %d size: %d\n", fbuf->pos, fbuf->size);

	first_part_size = min_int(MAX_FIFO_BUFFER_CAPACITY - fbuf->pos, requested_bytes);
	memcpy(result, fbuf->buffer + fbuf->pos, first_part_size);
//	printf("READ COPY: %d - %d\n" , fbuf->pos, first_part_size);

	if ( first_part_size < requested_bytes ) {
		/* We have overflow over the buffer end => handle the rest */
		memcpy(result + first_part_size, fbuf->buffer, requested_bytes - first_part_size);
//		printf("READ COPY: %d - %d\n" , 0, requested_bytes - first_part_size);
	}

	res = *result_length = requested_bytes;
	
	fbuf->size -= requested_bytes;

	if ( fbuf->size == 0 ) /* Just a quick check that can help us avoid unneccessary buffer splits in some cases */
		fbuf->pos = 0;
	else
		fbuf->pos = ( fbuf->pos + requested_bytes ) % MAX_FIFO_BUFFER_CAPACITY;

done:
	pthread_mutex_unlock(&fbuf->lock);
	return res;
}
