#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PAGE_SIZE	4096
#define NR_BUFF		1

/*
 * Touch first byte of each 4KB region successively
 * from both the buffers.
 */
static void access_buffers(char *b1, char *b2)
{
	char *start = b1, *start2 = b2;
	char *end = b1 + (1<<30), *end2 = b2 + (1<<30);
	char var;

	while (start < end) {
		*start = 'A';
		var = *start;
		start += PAGE_SIZE;
		*start2 = 'A';
		var = *start2;
		start2 += PAGE_SIZE;
	}
}

/*
 * Touch pages to force page allocation.
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

int main(int *argc, char *argv[])
{
	char *buff1[NR_BUFF], *buff2[NR_BUFF];
	int i, j, iteration;

	iteration = atoi(argv[1]);
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
	printf("Pages are ready to be accessed.\n");

	for (i=1; i<=iteration; i++) {
		if (i%1000 == 0)
			printf("Iteration Completed: %d\n", i);

		for (j=0; j < NR_BUFF; j++)
			access_buffers(buff1[j], buff2[j]);
	}
	return 0;
}
