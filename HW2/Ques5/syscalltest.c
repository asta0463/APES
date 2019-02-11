#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <syscall.h>

#define SYSCALL_NUM_HEAPSORT    398
#define BUFFER_SIZE 256

void main() {
        long retval;
        int32_t unsorted[BUFFER_SIZE];
        int32_t sorted[BUFFER_SIZE];
	int i;

        srandom(time(0));

        for (i=0; i<BUFFER_SIZE; i++) {
                unsorted[i]=((u_int32_t) random())%1000;
        }

	retval = syscall(SYSCALL_NUM_HEAPSORT, unsorted, sorted, (int32_t) BUFFER_SIZE);

	printf("\nReturn value: %ld", retval);
	

	for (i=0; i<BUFFER_SIZE; i++) {
                unsorted[i]=((u_int32_t) random())%1000;
        }

	retval = syscall(SYSCALL_NUM_HEAPSORT, unsorted, sorted, (int32_t) BUFFER_SIZE);
	
	printf("\nReturn value: %ld", retval);


	printf("\nPassing a NULL for the destination address");

	retval = syscall(SYSCALL_NUM_HEAPSORT, unsorted, NULL, (int32_t) BUFFER_SIZE);
	printf("\nReturn value: %ld", retval);


	printf("\nPassing a NULL for the source address");

        retval = syscall(SYSCALL_NUM_HEAPSORT, NULL, sorted, (int32_t) BUFFER_SIZE);
        printf("\nReturn value: %ld", retval);


        printf("\nPassing a size less than 256");

        retval = syscall(SYSCALL_NUM_HEAPSORT, unsorted, sorted, (int32_t) (BUFFER_SIZE-10));
        printf("\nReturn value: %ld", retval);

}
