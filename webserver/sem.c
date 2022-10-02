#include "sem.h"
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

// Thread implementation is based on:
// https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html

//Keep the pthreads inside of the struct
typedef struct SEM {
	int count;
	pthread_mutex_t thread_mutex;
	pthread_cond_t condition_cond;
} SEM;

SEM *sem_init(int initVal) {
	SEM* sem;
	sem = (SEM*) malloc(sizeof(struct SEM));
	if (sem == NULL) {
		printf("Could not create semaphore \n");
		return NULL;
	} 
	else {
		sem->count = initVal;
		int res1 = pthread_mutex_init(&(sem->thread_mutex), NULL);
		if(res1 != 0) {
			free(sem);
			return NULL;
		}
		int res2 = pthread_cond_init(&(sem->condition_cond), NULL);
		if(res2 != 0) {
			pthread_mutex_destroy(&(sem->thread_mutex));
			free(sem);
			return NULL;
		}
		return sem;
	}
}

int sem_del(SEM *sem) {
	int ret1 = pthread_mutex_destroy(&(sem->thread_mutex));
	if(ret1 != 0) {
		free(sem);
		return -1;
	}
	int ret2 = pthread_cond_destroy(&(sem->condition_cond));
	if(ret2 != 0) {
		free(sem);
		return -1;
	}
	return 0;
}



/*
Thought process: The SEM count can not be less than 0. If the count is zero
we block the thread, and wait till it is larger than zero. A condition
that only will occur if another thread calls V.
*/
void P(SEM *sem) {
	//Block the thread
	pthread_mutex_lock(&sem->thread_mutex);

	while(sem->count == 0){
		pthread_cond_wait(&sem->condition_cond, &sem->thread_mutex);
	}
	if(sem->count>0) {
		sem->count--;
	}

	//Unblock the thread
	pthread_mutex_unlock(&sem->thread_mutex);

}

void V(SEM *sem) {
	pthread_mutex_lock(&sem->thread_mutex);

	sem->count++;
	pthread_cond_signal(&sem->condition_cond);

	pthread_mutex_unlock(&sem->thread_mutex);
}