/**
* @file - pthreadtest.c
* @brief - Implementation file for 2 threads concurrently logging to a text file
* 
* @author - Ashish Tak
* @date 02/27/2019
**/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>

#define ALPHA_ASCII_OFFSET 65
#define ALPHA_COUNT 26


//Structure to pass the filename obtained from command line
struct parameter {
	char *filename;
};

uint32_t recdSignal=0, killedThreadID=0;


/*
 * Common signal handler for the defined user signals
 */
void sigHandler (int signal) {
	printf("Entered signal handler\n");
	killedThreadID= (pid_t)syscall(SYS_gettid);
	switch (signal) {
		case SIGUSR1:
			recdSignal=SIGUSR1;
			break;

		case SIGUSR2: 
			recdSignal=SIGUSR2;
			break;

		default:
			recdSignal=0;
			break;
	}
}


/*
 * Function to flush the character counts from the read file and close the handle of the file being written
 */
static void flushAndClose(FILE *fwrite, uint32_t *charCounts) {
	uint8_t i;
	char buf[100];
	struct timespec rtclk_time = {0, 0};
	for (i=0; i<ALPHA_COUNT; i++) {
		if(charCounts[i]<100 && charCounts[i]>0) {
	                sprintf(buf, "%c:%d\n", (char)i+ALPHA_ASCII_OFFSET, charCounts[i]);
                	fputs(buf, fwrite);
        	}
	}
        clock_gettime(CLOCK_REALTIME, &rtclk_time);
        sprintf(buf,"End Time for Character Processing Thread: sec=%ld, nanosec=%ld\n",\
                        rtclk_time.tv_sec, rtclk_time.tv_nsec);
        fputs(buf,fwrite);

	fflush(fwrite);
        fclose(fwrite);
}


/*
 * Start routine for the thread doing the processing of the characters
 */
static void *processCharacters (void *arg) {
	uint32_t charCounts[ALPHA_COUNT]={0};
	char ip, buf[100];
	struct parameter *para=arg;
	FILE *fwrite, *fread;
	struct timespec rtclk_time = {0, 0};

	printf("\nCharacter processing thread writing to file: %s\n", para->filename);

	fwrite=fopen(para->filename, "a");
	clock_gettime(CLOCK_REALTIME, &rtclk_time);
        printf("Character Processing thread started, PID %d TID %d\n", \
                                getpid(), (pid_t)syscall(SYS_gettid));
	sprintf(buf,"Character Processing thread started, PID %d TID %d\n", \
                                getpid(), (pid_t)syscall(SYS_gettid));
	fputs(buf,fwrite);
	sprintf(buf,"Start Time for Character Processing Thread: sec=%ld, nanosec=%ld\n",\
			rtclk_time.tv_sec, rtclk_time.tv_nsec);
	fputs(buf,fwrite);

	fread=fopen("gdb.txt", "r");
	while (!feof(fread)) {
		ip=fgetc(fread);
		if (isalpha(ip))
			//Update the counter array by hashing the values of the alphabets to the array indices
			++charCounts[toupper(ip)-ALPHA_ASCII_OFFSET];
		if (killedThreadID==(pid_t)syscall(SYS_gettid)) {
			if (recdSignal==SIGUSR1)
				fputs("Character Processing thread teminating with SIGUSR1\n", fwrite);
			else if (recdSignal==SIGUSR2)
				fputs("Character Processing thread teminating with SIGUSR2\n", fwrite);
			//Free all file pointers and flush the output
			fflush(fwrite);
			fclose(fread);
			flushAndClose(fwrite, charCounts);
			return NULL;
		}
	}
	fputs("Character Processing thread terminating by completion\n", fwrite);
	fflush(fwrite);
	fclose(fread);
	flushAndClose(fwrite, charCounts);
	return NULL;
}


volatile uint8_t CPUUtilFlag=0;


/*
 * Timer handler function for the 100 ms timeout of the CPU Utilization Reporting thread
 */
static void timerHandler(int sig) {
	//Set the flag to indicate writing to the log file on expiration of the 100ms timer
	CPUUtilFlag=1;
}


/*
 * Read the /proc/stat file and report the CPU utilization to the log file
 */
void checkStatFile (FILE *fwrite) {
	FILE *fread;
	uint8_t len=0;
	char *line=NULL, buf[100];
//	uint32_t user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
	fread=fopen("/proc/stat","r");
	getline(&line, &len, fread);
	/*
        sscanf(line, "%s  %d %d %d %d %d %d %d %d %d %d", str,\
			&user, &nice, &system, &idle, &iowait, &irq, \
			&softirq, &steal, &guest, &guest_nice);
	uint32_t totalUtil= user + nice + system + irq + softirq + steal;
	*/
	sprintf(buf,"CPU Utilization: %s\n",line);
	fputs(buf, fwrite);
}


/*
 * Start routine for the CPU Utilization Reporting thread
 */
static void *reportCPUUtil (void *arg) {
	char buf[100];
	struct parameter *para=arg;
	FILE *fwrite;
	struct timespec rtclk_time = {0, 0};

	//Posix Timer Initializations 
	timer_t timerid;
	struct itimerspec its;
	struct sigevent sev;
	struct sigaction tsa;

	tsa.sa_flags = 0;
	sigemptyset (&tsa.sa_mask);
	tsa.sa_handler=&timerHandler;
	sigaction(SIGRTMIN, &tsa, NULL);

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGRTMIN;
	sev.sigev_value.sival_ptr = &timerid;

	timer_create(CLOCK_REALTIME, &sev, &timerid);
	
	its.it_value.tv_sec=0;
	its.it_value.tv_nsec=100000000;
	its.it_interval.tv_sec =0;
	its.it_interval.tv_nsec = 100000000;

	timer_settime(timerid, 0, &its, NULL);


	printf("\nCPU Reporting thread writing to file: %s\n", para->filename);

	fwrite=fopen(para->filename, "a");
	clock_gettime(CLOCK_REALTIME, &rtclk_time);
        printf("CPU Utilization thread started, PID %d TID %d\n", \
                        getpid(), (pid_t)syscall(SYS_gettid));
        sprintf(buf,"CPU Utilization thread started, PID %d TID %d\n", \
                        getpid(), (pid_t)syscall(SYS_gettid));
	fputs(buf, fwrite);
        sprintf(buf,"Start Time for CPU Utilization Thread: sec=%ld, nanosec=%ld\n",\
                        rtclk_time.tv_sec, rtclk_time.tv_nsec);
        fputs(buf,fwrite);

	
        while (1) {
		//Check if a kill signal was issued for this particular thread
                if (killedThreadID==(pid_t)syscall(SYS_gettid)) {
                        if (recdSignal==SIGUSR1)
                                fputs("CPU utilization thread teminating with SIGUSR1\n", fwrite);
                        else if (recdSignal==SIGUSR2)
				fputs("CPU utilization thread teminating with SIGUSR2\n", fwrite);
                        //Free all file pointers
			clock_gettime(CLOCK_REALTIME, &rtclk_time);
		        sprintf(buf,"End Time for CPU Utilization Thread: sec=%ld, nanosec=%ld\n",\
					rtclk_time.tv_sec, rtclk_time.tv_nsec);
        		fputs(buf,fwrite);
			fflush(fwrite);
			fclose(fwrite);
                        return NULL;
                }
		if (CPUUtilFlag==1) {
			checkStatFile(fwrite);
			CPUUtilFlag=0;
		}
        }
	fputs("CPU Utilization thread terminating by completion\n", fwrite);
	fflush(fwrite);
	fclose(fwrite);
        return NULL;
}


void main (int argc, char **argv) {
	struct sigaction sa;
	sigemptyset (&sa.sa_mask);
	sa.sa_handler=&sigHandler;
	sa.sa_flags=0;
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);

	//Save the command line argument for filename 
	struct parameter p1;
	p1.filename=argv[1];
	
	pthread_t processChar, cpuUtil;
	pthread_create(&processChar, NULL, processCharacters, &p1);
	pthread_create(&cpuUtil, NULL, reportCPUUtil, &p1);

//	sleep(10);
//	pthread_kill(cpuUtil, SIGUSR1);

	pthread_join(processChar, NULL);
	pthread_join(cpuUtil, NULL);
	printf("\nThreads joined. Now Exiting...\n");
}
