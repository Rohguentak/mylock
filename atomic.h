#ifndef _ATOMIC_H_
#define _ATOMIC_H_

/**
 * Set *@value to @new if *@value was @old.
 * Return the old value of *@value
 *
 * *value  old  new  -->  return *value
 *      0   0    0             0   0
 *      0   0    1             0   1
 *      0   1    0             0   0
 *
 *      1   1    1             1   1
 *      1   1    0             1   0
 *      1   0    0             1   1
 */
static inline int compare_and_swap(volatile int *value, int old, int new)
{
	asm volatile (
		"lock ; cmpxchg %3, %1"
			: "=a"(old), "=m"(*value)
			: "a"(old), "r"(new)
			: "memory" );
	return old;
}
#endif
