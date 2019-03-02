#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>

#define STR_MAX_LEN 20


volatile uint8_t terminateSignal=0;

/*
 * Signal handler for the SIGINT signal
 */
void sigHandler (int signal) {
	switch (signal) {
		case SIGINT:
			terminateSignal=1;
			break;

		default:
			break;
	}
}


void printInfo(FILE *fp) {
	char buf[100];
       	sprintf(buf,"PID %d;  IPC method: Names pipes/ FIFO buffers  Resources: fifo1 and fifo2\
			\n",getpid());
	fputs(buf, fp);
	fflush(fp);
}



void main() {
	char *fifo1= "fifo1";
	char *fifo2= "fifo2";
	mkfifo(fifo1, 0666);
	mkfifo(fifo2, 0666);
	int fptr1, fptr2, i; 
	pid_t pid;

	FILE *fp;

	struct timespec rtclk_time = {0, 0};

	//Messages to be transmitted
	char txParent[5][STR_MAX_LEN]= {"Firmino", "Salah", "Mane", "Van Dijk", "Alisson"};
	char txChild[5][STR_MAX_LEN]= {"Fabinho", "Robertson", "Alexander Arnold", "Lovren",\
		"Wijnaldum"};
	uint8_t commands[5]= {1, 0, 1, 1, 0}; //ON/OFF signals
	char rxString[STR_MAX_LEN], buf[100];
	uint8_t read_cmd;

	struct sigaction sa;
	sigemptyset (&sa.sa_mask);
	sa.sa_handler=&sigHandler;
	sa.sa_flags=0;
	sigaction(SIGINT, &sa, NULL);

	pid=fork();

	//Parent
	if (pid!=0) {
		fptr1= open(fifo1, O_WRONLY);
		fptr2= open(fifo2, O_RDONLY);
		fp=fopen("output1.txt", "a");
		printInfo(fp);
		for (i=0; i<5; i++) {
			write(fptr1, &(commands[i]), 1);
			clock_gettime(CLOCK_REALTIME, &rtclk_time);
			sprintf(buf,"[sec=%ld, nanosec=%ld] PARENT SENDING: %d\n",\
					rtclk_time.tv_sec, rtclk_time.tv_nsec, commands[i]);
			fputs(buf,fp);
        	}
		for (i=0; i<5; i++) {
                        write(fptr1, txParent[i], sizeof(txParent[i]));
                        clock_gettime(CLOCK_REALTIME, &rtclk_time);
                        sprintf(buf,"[sec=%ld, nanosec=%ld] PARENT SENDING: %s\n",\
                                        rtclk_time.tv_sec, rtclk_time.tv_nsec, txParent[i]);
                        fputs(buf,fp);
		}
		fflush(fp);

		//Now read the data written by the child
		for (i=0; i<5; i++) {
			read(fptr2, &read_cmd, 1);
                        clock_gettime(CLOCK_REALTIME, &rtclk_time);
                        sprintf(buf,"[sec=%ld, nanosec=%ld] PARENT RECEIVED: %d\n",\
                                        rtclk_time.tv_sec, rtclk_time.tv_nsec, read_cmd);
                        fputs(buf,fp);
		}
		for (i=0; i<5; i++) {
			read(fptr2, rxString, STR_MAX_LEN);
                        clock_gettime(CLOCK_REALTIME, &rtclk_time);
                        sprintf(buf,"[sec=%ld, nanosec=%ld] PARENT RECEIVED: %s\n",\
                                        rtclk_time.tv_sec, rtclk_time.tv_nsec, rxString);
                        fputs(buf,fp);
		}
		fflush(fp);

		//Wait until the termination signal is received
		while(!terminateSignal);
		close (fptr1);
		close (fptr2);
                clock_gettime(CLOCK_REALTIME, &rtclk_time);
		sprintf(buf,"[sec=%ld, nanosec=%ld] SIGTERM Signal Received, Parent Exiting\n",\
				rtclk_time.tv_sec, rtclk_time.tv_nsec);
		fputs(buf,fp);
		fflush(fp);
		fclose(fp);
	}

	//Child
	else {
		fptr1= open(fifo1, O_RDONLY);
		fptr2= open(fifo2, O_WRONLY);
                fp=fopen("output1.txt", "a");
		printInfo(fp);
                for (i=0; i<5; i++) {
                        write(fptr2, &(commands[i]), 1);
                        clock_gettime(CLOCK_REALTIME, &rtclk_time);
                        sprintf(buf,"[sec=%ld, nanosec=%ld] CHILD SENDING: %d\n",\
                                        rtclk_time.tv_sec, rtclk_time.tv_nsec, commands[i]);
                        fputs(buf,fp);
                }
                for (i=0; i<5; i++) {
                        write(fptr2, txChild[i], sizeof(txChild[i]));
                        clock_gettime(CLOCK_REALTIME, &rtclk_time);
                        sprintf(buf,"[sec=%ld, nanosec=%ld] CHILD SENDING: %s\n",\
                                        rtclk_time.tv_sec, rtclk_time.tv_nsec, txChild[i]);
                        fputs(buf,fp);
                }
		fflush(fp);

                //Now read the data written by the parent
                for (i=0; i<5; i++) {
                        read(fptr1, &read_cmd, 1);
                        clock_gettime(CLOCK_REALTIME, &rtclk_time);
                        sprintf(buf,"[sec=%ld, nanosec=%ld] CHILD RECEIVED: %d\n",\
                                        rtclk_time.tv_sec, rtclk_time.tv_nsec, read_cmd);
                        fputs(buf,fp);
                }
                for (i=0; i<5; i++) {
                        read(fptr1, rxString, STR_MAX_LEN);
                        clock_gettime(CLOCK_REALTIME, &rtclk_time);
                        sprintf(buf,"[sec=%ld, nanosec=%ld] CHILD RECEIVED: %s\n",\
                                        rtclk_time.tv_sec, rtclk_time.tv_nsec, rxString);
                        fputs(buf,fp);
                }
		fflush(fp);

		while(!terminateSignal);
                close (fptr1);
                close (fptr2);
                clock_gettime(CLOCK_REALTIME, &rtclk_time);
                sprintf(buf,"[sec=%ld, nanosec=%ld] SIGTERM Signal Received, Child Exiting\n",\
				rtclk_time.tv_sec, rtclk_time.tv_nsec);
                fputs(buf,fp);
                fflush(fp);
		fclose(fp);
	}
}
