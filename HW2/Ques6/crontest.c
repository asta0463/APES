#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <syscall.h>

#define SYSCALL_NUM_HEAPSORT    398
#define SYSCALL_GETUID	24
#define SYSCALL_GETPID	20
#define BUFFER_SIZE 256

void main() {
        long retval;
        int32_t unsorted[BUFFER_SIZE];
        int32_t sorted[BUFFER_SIZE];
	struct timeval tv;
	struct timezone *tz=NULL;

        srandom(time(0));

        for (int i=0; i<BUFFER_SIZE; i++) {
                unsorted[i]=((u_int32_t) random())%1000;
                //printf("%d\n",unsorted[i]);
        }
	
	printf("\nPeriodic CRON Task\n");

	pid_t pid= syscall(SYSCALL_GETPID);
	printf("\nProcess ID: %d", pid);

	uid_t uid=syscall(SYSCALL_GETUID);
	printf("\nUser ID: %d", uid);

	gettimeofday(&tv, tz);
	char *datetime=ctime(&tv.tv_sec);
	printf("\nDate-Time: %s", datetime);

        retval = syscall(SYSCALL_NUM_HEAPSORT, unsorted, sorted, (int32_t) BUFFER_SIZE);

        printf("\nReturn value from system call: %ld", retval);
}
