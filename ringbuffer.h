#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

enum lock_types;

int init_ringbuffer(const int nr_slots, const enum lock_types lock_type);
void fini_ringbuffer(void);

void enqueue_ringbuffer(int value);
int dequeue_ringbuffer(void);

#endif
