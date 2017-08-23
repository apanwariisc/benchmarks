#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#define PAGE_SIZE	4096
#define NR_BUFF		1

#define MODE_READ_ONLY	0
#define MODE_WRITE_ONLY 1
#define MODE_READ_WRITE	2
#define MODE_WRITE_READ	3

/*
 * Touch first byte of each 4KB region successively
 * from both the buffers. We do not care about address
 * alignment here as PAZE_SIZE would always ensure we are
 * actually moving to the next physical page.
 */
static void access_buffers(char *b1, char *b2, int mode)
{
	char *start = b1;
	char *end = b1 + (1<<30);
	char var;
	int repeated = 0;

repeat:
	while (start < end) {
		if (start == b2)
			printf("Starting with second buffer\n");
		if (mode == MODE_READ_ONLY)
			var = *start;
		else if (mode == MODE_WRITE_ONLY)
			*start = 'A';
		else if (mode == MODE_READ_WRITE) {
			var = *start;
			*start = 'A';
		}
		else {
			*start = 'A';
			var = *start;
		}
		start += PAGE_SIZE;
	}
	/* Prepare pointers to traverse the second buffer.*/
	start = b2;
	end = b2 + (1<<30);
	
	/* If both the buffers are traversed, return. */
	if (repeated)
		return;
	
	repeated = 1;
	goto repeat;

	/* Should never reach here. */
	assert(0);
}

/*
 * Touch pages to force page allocation and warm the cache.
 */
static void do_fault_buffer(char *buff)
{
	char *start = buff;
	char *end = buff + (1<<30);

	while (start < end) {
		*start = 'A';
		start += PAGE_SIZE;
	}
}

int main(int argc, char *argv[])
{
	char *buff1[NR_BUFF], *buff2[NR_BUFF];
	char mode_str[4][12] = {"READ_ONLY\0","WRITE_ONLY\0",
					"READ_WRITE\0","WRITE_READ\0"};
	int i, j, iteration, mode = -1;

	if (argc != 3) {
		printf("Usage: ./sequential mode iteration\n");
		exit(1);
	}

	/* Accept and parse arguments.*/
	mode = atoi(argv[1]);
	iteration = atoi(argv[2]);
	assert(mode >= 0 && mode <= 3);

	/* Allocate empty buffers */
	for (i = 0; i < NR_BUFF; i++) {
		buff1[i] = malloc(1<<30);
		buff2[i] = malloc(1<<30);
		if (!buff1[i] || !buff2[i]) {
			printf("Buffer allocation failed\n");
			exit(1);
		}
	}

	printf("Buffer are ready to be used.\n");
	printf("Generating faults to allocate pages.\n");
	for (i = 0; i < NR_BUFF; i++) {
		do_fault_buffer(buff1[i]);
		do_fault_buffer(buff2[i]);
	}

	printf("Accessing buffers in %s mode\n", mode_str[mode]);
	for (i=1; i<=iteration; i++) {
		if (i%1000 == 0)
			printf("Iteration Completed: %d\n", i);

		for (j=0; j<NR_BUFF; j++) {
			access_buffers(buff1[j], buff2[j], mode);
		}
	}
	return 0;
}
