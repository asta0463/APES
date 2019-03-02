#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <semaphore.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <sys/shm.h>

#define STR_MAX_LEN 20
#define SHM_SEGMENT_SIZE 65536
#define SHM_SEGMENT_NAME "/shared_mem"
#define SEM_READ "/shm_read"
#define SEM_WRITE "/shm_write"

static sem_t *shm_read, *shm_write;
int shm_fd;

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


/*
* If the shared memory segment does not exist already, create it
* Returns a pointer to the segment or NULL if there is an error
*/
static void *get_shared_memory(void) {
	struct shared_data *shm_p;
	/* Attempt to create the shared memory segment */
	shm_fd = shm_open(SHM_SEGMENT_NAME, O_CREAT | O_EXCL | O_RDWR,0666);
	if (shm_fd > 0) {
		/* succeeded: expand it to the desired size (Note: dont't do
		 * this every time because ftruncate fills it with zeros) */
		printf ("Creating shared memory and setting size=%d\n", \
				SHM_SEGMENT_SIZE);
		if (ftruncate(shm_fd, SHM_SEGMENT_SIZE) < 0) {
			perror("ftruncate");
			exit(1);
		}

		/* Create read and write semaphores as well */
		shm_read = sem_open(SEM_READ, O_RDWR | O_CREAT, 0666, 0);
		shm_write = sem_open(SEM_WRITE, O_RDWR | O_CREAT, 0666, 1);
		if (shm_read == SEM_FAILED || shm_write == SEM_FAILED)
			perror("sem_open failed\n");
	}
	else if (shm_fd == -1 && errno == EEXIST) {
		/* Already exists: open again without O_CREAT */
		shm_fd = shm_open(SHM_SEGMENT_NAME, O_RDWR, 0);
                shm_read = sem_open(SEM_READ, O_RDWR | O_CREAT, 0666, 0);
                shm_write = sem_open(SEM_WRITE, O_RDWR | O_CREAT, 0666, 1);
                if (shm_read == SEM_FAILED || shm_write == SEM_FAILED)
                        perror("sem_open failed\n");

	}
	if (shm_fd == -1) {
		perror("shm_open " SHM_SEGMENT_NAME);
		exit(1);
	}

	/* Map the shared memory */
	shm_p = mmap(NULL, SHM_SEGMENT_SIZE, PROT_READ | PROT_WRITE,\
			MAP_SHARED, shm_fd, 0);
	if (shm_p == NULL) {
		perror("mmap");
		exit(1);
	}

	return shm_p;
}



void main()
{
	char *shm_p;
	pid_t pid;
	FILE *fp;
	int i;
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


	shm_p = get_shared_memory();

	pid= fork();

	if (pid!=0) {
		fp=fopen("output3.txt", "a");

		sprintf(buf,"PID %d;  IPC:Shared mem;  Resources:/shared_mem and semaphores /shm_read & shm_write\n",getpid());
		fputs(buf, fp);
		fflush(fp);
		
		//Parent Writing

                for (i=0; i<5; i++) {
                        sem_wait(shm_write);
			*(uint8_t *)shm_p=commands[i];
                        sem_post(shm_read);
                        clock_gettime(CLOCK_REALTIME, &rtclk_time);
                        sprintf(buf,"[sec=%ld, nanosec=%ld] PARENT WRITING: %d\n",\
                                        rtclk_time.tv_sec, rtclk_time.tv_nsec, commands[i]);
                        fputs(buf,fp);
                }
		fflush(fp);

		for (i=0; i<5; i++) {
			sem_wait(shm_write);
			sprintf(shm_p, "%s", txParent[i]);
			sem_post(shm_read);
			clock_gettime(CLOCK_REALTIME, &rtclk_time);
			sprintf(buf,"[sec=%ld, nanosec=%ld] PARENT WRITING: %s\n",\
					rtclk_time.tv_sec, rtclk_time.tv_nsec, txParent[i]);
			fputs(buf,fp);
        	}
		fflush(fp);

		usleep(1);


                //Parent Reading

                for (i=0; i<5; i++) {
                        sem_wait(shm_read);
                        clock_gettime(CLOCK_REALTIME, &rtclk_time);
                        sprintf(buf,"[sec=%ld, nanosec=%ld] PARENT READING: %d\n",\
                                        rtclk_time.tv_sec, rtclk_time.tv_nsec, *((uint8_t *)shm_p));
                        sem_post(shm_write);
                        fputs(buf,fp);
                }
                fflush(fp);

                for (i=0; i<5; i++) {
                        sem_wait(shm_read);
                        clock_gettime(CLOCK_REALTIME, &rtclk_time);
                        sprintf(buf,"[sec=%ld, nanosec=%ld] PARENT READING: %s\n",\
                                        rtclk_time.tv_sec, rtclk_time.tv_nsec, shm_p);
                        sem_post(shm_write);
                        fputs(buf,fp);
                }
                fflush(fp);

		while(!terminateSignal);
                clock_gettime(CLOCK_REALTIME, &rtclk_time);
                sprintf(buf,"[sec=%ld, nanosec=%ld] SIGINT Signal Received, Parent Exiting\n",\
				rtclk_time.tv_sec, rtclk_time.tv_nsec);
                fputs(buf,fp);
		fflush(fp);
		fclose(fp);

		shmdt(SHM_SEGMENT_NAME);
		shmctl(shm_fd, IPC_RMID, NULL);
	}

	else 
	{
		fp=fopen("output3.txt", "a");

                sprintf(buf,"PID %d;  IPC:Shared mem;  Resources:/shared_mem and semaphores /shm_read & shm_write\n",getpid());

                fputs(buf, fp);
                fflush(fp);

		//Child Reading
		
                for (i=0; i<5; i++) {
                        sem_wait(shm_read);
                        clock_gettime(CLOCK_REALTIME, &rtclk_time);
                        sprintf(buf,"[sec=%ld, nanosec=%ld] CHILD READING: %d\n",\
                                        rtclk_time.tv_sec, rtclk_time.tv_nsec, *((uint8_t *)shm_p));
			sem_post(shm_write);
                        fputs(buf,fp);
                }
		fflush(fp);

                for (i=0; i<5; i++) {
                        sem_wait(shm_read);
                        clock_gettime(CLOCK_REALTIME, &rtclk_time);
                        sprintf(buf,"[sec=%ld, nanosec=%ld] CHILD READING: %s\n",\
                                        rtclk_time.tv_sec, rtclk_time.tv_nsec, shm_p);
                        sem_post(shm_write);
                        fputs(buf,fp);
                }
		fflush(fp);



               //Child Writing

                for (i=0; i<5; i++) {
                        sem_wait(shm_write);
                        *(uint8_t *)shm_p=commands[i];
                        sem_post(shm_read);
                        clock_gettime(CLOCK_REALTIME, &rtclk_time);
                        sprintf(buf,"[sec=%ld, nanosec=%ld] CHILD WRITING: %d\n",\
                                        rtclk_time.tv_sec, rtclk_time.tv_nsec, commands[i]);
                        fputs(buf,fp);
                }
                fflush(fp);

                for (i=0; i<5; i++) {
                        sem_wait(shm_write);
                        sprintf(shm_p, "%s", txChild[i]);
                        sem_post(shm_read);
                        clock_gettime(CLOCK_REALTIME, &rtclk_time);
                        sprintf(buf,"[sec=%ld, nanosec=%ld] PARENT WRITING: %s\n",\
                                        rtclk_time.tv_sec, rtclk_time.tv_nsec, txChild[i]);
                        fputs(buf,fp);
                }
		fflush(fp);

                while(!terminateSignal);
                clock_gettime(CLOCK_REALTIME, &rtclk_time);
                sprintf(buf,"[sec=%ld, nanosec=%ld] SIGINT Signal Received, Child Exiting\n",\
                                rtclk_time.tv_sec, rtclk_time.tv_nsec);
                fputs(buf,fp);
                fflush(fp);
                fclose(fp);

                shmdt(SHM_SEGMENT_NAME);
	}

	sem_unlink(SEM_READ);
	sem_unlink(SEM_WRITE);

}
