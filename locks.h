#ifndef __LOCKS_H__
#define __LOCKS_H__
enum lock_types;

/**
 * Have a look at https://linux.die.net/man/3/list_head for using list_head
 */
#include <sys/queue.h>
void sig_handler(int signum);
struct thread {
	int wake;
	pthread_t pid;
	unsigned long flags;
	TAILQ_ENTRY(thread) next;
};

/*************************************************
 * Spinlock
 */
struct spinlock {
	volatile int turn;
	
};
/*struct spinlock {
	pthread_mutex_t mutex;
};*/
void init_spinlock(struct spinlock *);
void acquire_spinlock(struct spinlock *);
void release_spinlock(struct spinlock *);


/*************************************************
 * Mutex
 */
struct mutex {
	struct spinlock sfm;
	volatile int value;
	TAILQ_HEAD(threads, thread) waiter;
	//int wak;
	/* Fill this in */
};
void init_mutex(struct mutex *);
void acquire_mutex(struct mutex *);
void release_mutex(struct mutex *);


/*************************************************
 * Semaphore
 */
struct semaphore {
	struct spinlock sfm;
	volatile int value;
	TAILQ_HEAD(threads2, thread) waiter;
	
	/* Fill this in */
};
void init_semaphore(struct semaphore *, const int);
void wait_semaphore(struct semaphore *);
void signal_semaphore(struct semaphore *);

/*************************************************
 * Lock tester.
 * Will be invoked if the program is run with -T
 */
void test_lock(void);
#endif
