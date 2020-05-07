#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "config.h"
#include "locks.h"
struct queue{
    int head;
    int tail;
    int size;
	int* arr;
	struct spinlock news;
};
struct queue2{
	int head;
    int tail;
    int size;
	int* arr;
	struct mutex newmu;
};
struct queue3{
	int head;
    int tail;
    int size;
	int* arr;
	struct semaphore mtx;
	struct semaphore empty;
	struct semaphore full;
};
struct queue newq; 
struct queue2 newq2; 
struct queue3 sema; 
static int nr_slots = 0;

static enum lock_types lock_type;

void (*enqueue_fn)(int value) = NULL;
int (*dequeue_fn)(void) = NULL;

void enqueue_ringbuffer(int value)
{
	assert(enqueue_fn);
	assert(value >= MIN_VALUE && value < MAX_VALUE);

	enqueue_fn(value);
}

int dequeue_ringbuffer(void)
{
	int value;

	assert(dequeue_fn);

	value = dequeue_fn(); //printf("value is %d\n",value);
	assert(value >= MIN_VALUE && value < MAX_VALUE);

	return value;
}


/*********************************************************************
 * TODO: Implement using spinlock
 */
void enqueue_using_spinlock(int value)
{
	again:
	acquire_spinlock(&(newq.news));
	if((newq.tail+1)%newq.size == newq.head){
		release_spinlock(&(newq.news));
		goto again;
	}
	newq.arr[newq.tail] = value; //printf("newq.arr[%d] = %d\n",newq.tail,value);
	newq.tail = (newq.tail+1)%newq.size; 
	release_spinlock(&(newq.news));
	
}

int dequeue_using_spinlock(void)
{	
	int retval =0; 
	again:
	acquire_spinlock(&(newq.news));
	if(newq.head == newq.tail){
		release_spinlock(&(newq.news));
		goto again;
	}
	retval = newq.arr[newq.head];
	newq.head = (newq.head+1)%newq.size; 
	release_spinlock(&(newq.news));
	return retval;
}

void init_using_spinlock(void)
{
	enqueue_fn = &enqueue_using_spinlock;
	dequeue_fn = &dequeue_using_spinlock;
}

void fini_using_spinlock(void)
{
	free(newq.arr);	
}


/*********************************************************************
 * TODO: Implement using mutex
 */
void enqueue_using_mutex(int value)
{	
	again:
	acquire_mutex(&(newq2.newmu));
	if((newq2.tail+1)%newq2.size == newq2.head){
		release_mutex(&(newq2.newmu));
		goto again;
	}
	else{
		newq2.arr[newq2.tail] = value; 
		newq2.tail = (newq2.tail+1)%newq2.size;
		release_mutex(&(newq2.newmu));
	}
}

int dequeue_using_mutex(void)
{
	int retval = 0;
again:	
	acquire_mutex(&(newq2.newmu));
	if(newq2.head == newq2.tail){
		release_mutex(&(newq2.newmu));
		goto again;
	}else{
		retval = newq2.arr[newq2.head];
		newq2.head = (newq2.head+1)%newq2.size; 
		release_mutex(&(newq2.newmu));
		return retval;
	}
}

void init_using_mutex(void)
{
	enqueue_fn = &enqueue_using_mutex;
	dequeue_fn = &dequeue_using_mutex;
}

void fini_using_mutex(void)
{
	free(newq2.arr);	
}


/*********************************************************************
 * TODO: Implement using semaphore
 */
void enqueue_using_semaphore(int value)
{
	
	wait_semaphore(&(sema.empty));
	again:
	wait_semaphore(&(sema.mtx));
	if((newq2.tail+1)%newq2.size == newq2.head){
		signal_semaphore(&(sema.mtx));
		goto again;
	}
	else{
		newq2.arr[newq2.tail] = value; 
		newq2.tail = (newq2.tail+1)%newq2.size;
		signal_semaphore(&(sema.mtx));
		signal_semaphore(&(sema.full));
		
	}
}

int dequeue_using_semaphore(void)
{
	int retval = 0;
	
	wait_semaphore(&(sema.full));
	again:
	wait_semaphore(&(sema.mtx));
	if(newq2.head == newq2.tail){
		signal_semaphore(&(sema.mtx));
		goto again;
	}else{
		retval = newq2.arr[newq2.head];
		newq2.head = (newq2.head+1)%newq2.size; 
		signal_semaphore(&(sema.mtx));
		signal_semaphore(&(sema.empty));
		return retval;
	}
}

void init_using_semaphore(void)
{
	enqueue_fn = &enqueue_using_semaphore;
	dequeue_fn = &dequeue_using_semaphore;
}

void fini_using_semaphore(void)
{
	free(sema.arr);
}


/*********************************************************************
 * Common implementation
 */
int init_ringbuffer(const int _nr_slots_, const enum lock_types _lock_type_)
{
	assert(_nr_slots_ > 0);
	nr_slots = _nr_slots_;

	/* Initialize lock! */
	lock_type = _lock_type_;
	switch (lock_type) {
	case lock_spinlock:
		init_using_spinlock();
		break;
	case lock_mutex:
		init_using_mutex();
		break;
	case lock_semaphore:
		init_using_semaphore();
		break;
	}
	/* TODO: Initialize your ringbuffer and synchronization mechanism */
	newq.size = nr_slots;
	newq.arr = malloc(sizeof(int)*newq.size);
	newq.head = 0; newq.tail = 0;
	init_spinlock(&(newq.news)); 
	newq2.size = nr_slots; //printf("size is %d \n",newq2.size);
	newq2.arr = malloc(sizeof(int)*newq2.size);// printf("arr value is %d\n",newq2.arr[0]);
	newq2.head = 0; newq2.tail = 0;
	init_mutex(&(newq2.newmu)); 
	sema.size = nr_slots; //printf("size is %d \n",newq2.size);
	sema.arr = malloc(sizeof(int)*sema.size);// printf("arr value is %d\n",newq2.arr[0]);
	sema.head = 0; sema.tail = 0;
	init_semaphore(&(sema.mtx),1);
	init_semaphore(&(sema.empty),nr_slots);
	init_semaphore(&(sema.full),0);
	return 0;
}

void fini_ringbuffer(void)
{
	/* TODO: Clean up what you allocated */
	switch (lock_type) {
	case lock_spinlock:
		fini_using_spinlock();
		break;
	case lock_mutex:
		fini_using_mutex();
		break;
	case lock_semaphore:
		fini_using_semaphore();
	default:
		break;
	}
}
