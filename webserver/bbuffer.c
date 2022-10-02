#include "bbuffer.h"
#include "sem.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct BNDBUF {
	int size;
	int* buffer;
	int next_in, next_out;
	SEM* empty; //Count empty slots in the buffer
	SEM* data; //Count number of data items in the buffer
	SEM* busy;
} BNDBUF;

BNDBUF *bb_init(unsigned int size) {
	BNDBUF* buffer;
	buffer = (BNDBUF*) malloc(sizeof(struct BNDBUF));
	if (buffer == NULL) {
		printf("Could not create buffer\n");
		free(buffer);
		return NULL;
	}
	int* bb = malloc(sizeof(int) * size);
	if(bb == NULL) {
		printf("Could not allocate internal list\n");
		free(buffer);
		return NULL;
	}
	buffer->empty = sem_init(size);
	if(buffer->empty == NULL) {
		free(buffer);
		free(bb);
		return NULL;
	}
	buffer->data = sem_init(0);
	if(buffer->data == NULL) {
		sem_del(buffer->empty);
		free(buffer);
		free(bb);
		return NULL;
	}
	buffer->busy = sem_init(1);
	if(buffer->busy == NULL) {
		sem_del(buffer->empty);
		sem_del(buffer->data);
		free(buffer);
		free(bb);
		return NULL;
	}

	buffer->size = size;
	buffer->buffer = bb;
	buffer->next_out = 0;
	buffer->next_in = 0;

	return buffer;
}

void bb_del(BNDBUF *bb) {
	sem_del(bb->busy);
	sem_del(bb->empty);
	sem_del(bb->data);
	free(bb);
}

int bb_get(BNDBUF *bb) {
	P(bb->data);
	P(bb->busy);
	int out = bb->next_out;
	int return_value = bb->buffer[out];

	bb->next_out = (bb->next_out + 1)%(bb->size);

	V(bb->busy);
	V(bb->empty);
	return return_value;
}

void bb_add(BNDBUF *bb, int fd) {
	P(bb->empty);
	P(bb->busy);
	int next_in = bb->next_in;
	bb->buffer[next_in] = fd;

	bb->next_in = (bb->next_in + 1)%(bb->size);

	V(bb->busy);
	V(bb->data);
}