#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <pthread.h>

#include "config.h"
#include "ringbuffer.h"
#include "generator.h"
#include "counter.h"
#include "locks.h"

/* Common */
int verbose = 1;

/* Generator */
static enum generator_types generator_type = generator_constant; //generator_type = 2;
static int nr_generators = DEFAULT_NR_GENERATORS;  //1
static unsigned long nr_generate = DEFAULT_NR_GENERATE;  //32

/* Counter */
static enum counter_types counter_type = counter_normal; //counter_type = 0

/* Ring buffer */
static int nr_slots = DEFAULT_NR_RINGBUFFER_SLOTS;  //16
static enum lock_types lock_type = lock_spinlock; //0

int parse_command(int argc, char *argv[])
{
	char opt;
	while ((opt = getopt(argc, argv, "vqg:s:n:R:rSM01234h?T")) != -1) {
		switch(opt) {
		case 'v':
			verbose = 1;
			break;
		case 'q':
			verbose = 0;
			break;
		case 'T':
			test_lock();
			exit(0);
		case 'R':
			srandom(atoi(optarg));
			break;
		case 'r':
			generator_type = generator_random;
			break;
		case 'g':
			nr_generators = atoi(optarg);
			break;
		case 'n':
			nr_generate = atoll(optarg);
			break;
		case 'S':
			lock_type = lock_semaphore;
			break;
		case 'M':
			lock_type = lock_mutex;
			break;
		case 's':
			nr_slots = atoi(optarg);
			break;
		case '0':
			generator_type = generator_random;
			nr_generators = 4;
			nr_generate = (1 << 12);
			lock_type = lock_spinlock;
			verbose = 0;
			break;
		case '1':
			generator_type = generator_random;
			nr_generators = 4;
			nr_generate = (1 << 12);
			lock_type = lock_mutex;
			verbose = 0;
			break;
		case '2':
			generator_type = generator_random;
			nr_generators = 4;
			nr_generate = (1 << 12);
			lock_type = lock_semaphore;
			verbose = 0;
			break;
		case '3': /* Overflow test */
			generator_type = generator_random;
			nr_generators = 4;
			nr_generate = (1 << 10);
			counter_type = counter_delayed;
			verbose = 0;
			break;
		case '4': /* Underflow test */
			generator_type = generator_delayed;
			nr_generators = 4;
			nr_generate = (1 << 10);
			verbose = 0;
			break;
		case 'h':
		case '?':
		default:
			fprintf(stderr, "Usage: %s [-g generators] \n", argv[0]);
			fprintf(stderr, "             [-n # of request for a generator to generate]\n");
			fprintf(stderr, "             [-s ringbuffer slots]\n");
			fprintf(stderr, "             [-v | -q] /* Make verbose or quiet */\n");
			fprintf(stderr, "             [-r]      /* Use random generator */\n");
			fprintf(stderr, "             [-M]      /* Test mutex */\n");
			fprintf(stderr, "             [-S]      /* Test semaphore */\n");
			fprintf(stderr, "             [-h | -?] /* Print usage */\n");
			return -EINVAL;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int retval = 0;

	system("rm -f GEN* RESULT*"); //GEN file and RESULT FILE remove;

	if ((retval = parse_command(argc, argv))) {
		goto exit;
	}
	if ((retval = init_ringbuffer(nr_slots, lock_type))) {
		goto exit;
	}

	if ((retval = spawn_counter(counter_type, nr_generators * nr_generate))) {
		goto exit_ring;
	}

	spawn_generators(generator_type, nr_generators, nr_generate);

	fini_generators();
	fini_counter();

exit_ring:
	fini_ringbuffer();
exit:
	return retval;
}
