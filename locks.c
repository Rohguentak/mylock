//#define _XOPEN_SOURCE 700
//#define _XOPEN_SOURCE_EXTENDED
//#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "config.h"
#include "locks.h"
#include "atomic.h"
//int wak=0;
void sig_handler(int signum){
	if(signum == SIGUSR1)
	return ;
}
/******************************************************************
 * Spinlock implementation
 */
void init_spinlock(struct spinlock *lock)
{
	lock->turn = 0;
	return;
}

void acquire_spinlock(struct spinlock *lock)
{
	while(compare_and_swap(&(lock->turn),0,1));
	return;
}

void release_spinlock(struct spinlock *lock)
{
	lock->turn = 0;
	return;
}



/******************************************************************
 * Blocking lock implementation
 *
 * Hint: Use pthread_self, pthread_kill, pause, and signal
 */
void init_mutex(struct mutex *lock)
{
	lock->sfm.turn = 0;
	lock->value = 1;
	TAILQ_INIT(&lock->waiter);
	return;
}

void acquire_mutex(struct mutex *lock)
{	
	signal(SIGUSR1,sig_handler);
	acquire_spinlock(&(lock->sfm));	
	lock->value--;
	if(lock->value < 0){
		struct thread *t;
		t = malloc(sizeof(*t));
		t->pid = pthread_self();
		t->wake =0;
	
		TAILQ_INSERT_TAIL(&lock->waiter,t,next); 
		release_spinlock(&(lock->sfm));
		pause();
		t->wake =1;
	}
	else{
		release_spinlock(&(lock->sfm));
	}
	return;
}

void release_mutex(struct mutex *lock)
{
	acquire_spinlock(&(lock->sfm));
	lock->value++;
	if(lock->value<=0){
		struct thread *t = lock->waiter.tqh_first;
		TAILQ_REMOVE(&lock->waiter,t,next);
		do{
		pthread_kill(t->pid,SIGUSR1);
		usleep(100);	
		}while(t->wake ==0);
		t->wake =0;
		free(t);
	}	
	release_spinlock(&(lock->sfm));
	return;
}


/******************************************************************
 * Semaphore implementation
 *
 * Hint: Use pthread_self, pthread_kill, pause, and signal
 */
void init_semaphore(struct semaphore *sem, int S)
{
	sem->sfm.turn = 0;
	sem->value = S;
	TAILQ_INIT(&sem->waiter);
	
	return;
}

void wait_semaphore(struct semaphore *sem)
{
	signal(SIGUSR1,sig_handler);
	acquire_spinlock(&(sem->sfm));	
	sem->value--;
	if(sem->value < 0){
		struct thread *t;
		t = malloc(sizeof(*t));
		t->pid = pthread_self();
		t->wake =0;
	
		TAILQ_INSERT_TAIL(&sem->waiter,t,next); 
		release_spinlock(&(sem->sfm));
		pause();
		t->wake =1;
	}
	else{
		release_spinlock(&(sem->sfm));
	}
	return;
}

void signal_semaphore(struct semaphore *sem)
{
	acquire_spinlock(&(sem->sfm));
	sem->value++;
	if(sem->value<=0){
		struct thread *t = sem->waiter.tqh_first;
		TAILQ_REMOVE(&sem->waiter,t,next);
		do{
		pthread_kill(t->pid,SIGUSR1);
		usleep(100);	
		}while(t->wake ==0);
		t->wake =0;
		free(t);
	}	
	release_spinlock(&(sem->sfm));
	return;
}


/******************************************************************
 * Spinlock tester exmaple
 */
//struct spinlock testlock;
struct mutex testlock;
//int testlock_held = 0;

void *test_thread(void *_arg_)
{
	usleep(random() % 1000 * 1000);

	printf("Tester acquiring the lock...\n");
	acquire_mutex(&testlock);
	printf("Tester acquired\n");
	assert(testlock.value <= 0);
	sleep(1);
	printf("Tester releases the lock\n");
	release_mutex(&testlock);
	assert(testlock.value <= 1);
	
	printf("Tester released the lock\n");
	return 0;
}

void test_lock(void)
{
	/* Set nr_testers as you need
	 *  1: one main, one tester. easy :-)
	 * 16: one main, 16 testers contending the lock :-$
	 */
	const int nr_testers = 16;
	int i;
	pthread_t tester[nr_testers];

	printf("Main initializes the lock\n");
	init_mutex(&testlock);
	printf("Main graps the lock...");
	acquire_mutex(&testlock);
	assert(testlock.value <= 0);
	printf("acquired!\n");
	release_mutex(&testlock);
	for (i = 0; i < nr_testers; i++) {
		pthread_create(tester + i, NULL, test_thread, NULL);
	}
	sleep(1);
	printf("Main releases the lock\n");
	
	printf("Main released the lock\n");
	for (i = 0; i < nr_testers; i++) {
		pthread_join(tester[i], NULL);
	}
	assert(testlock.value == 1);
	printf("Your spinlock implementation looks O.K.\n");

	return;
}

