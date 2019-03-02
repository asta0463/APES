#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>

#define SOCKET_NAME "local.socket"
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


void main()
{
   struct sockaddr_un addr;
   int i, data_socket;
   uint8_t result;
   char buffer[STR_MAX_LEN], buf[100];
   
   FILE *fp;
   struct timespec rtclk_time = {0, 0};

   //Messages to be transmitted by the client
   char txClient[5][STR_MAX_LEN]= {"Firmino", "Salah", "Mane", "Van Dijk", "Alisson"};
   uint8_t commands[5]= {1, 0, 1, 1, 0}; //ON/OFF signals

   struct sigaction sa;
   sigemptyset (&sa.sa_mask);
   sa.sa_handler=&sigHandler;
   sa.sa_flags=0;
   sigaction(SIGINT, &sa, NULL);

   /* Create local socket. */
   data_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);

   /* Clear the whole structure */
   memset(&addr, 0, sizeof(struct sockaddr_un));

   /* Connect socket to socket address */
   addr.sun_family = AF_UNIX;
   strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);
   connect (data_socket, (const struct sockaddr *) &addr,
				  sizeof(struct sockaddr_un));

   fp=fopen("output2.txt", "a");
   
   sprintf(buf,"PID %d;  IPC method: Sockets  Resource: localsocket\n",getpid());
   fputs(buf, fp);
   fflush(fp);
   
   /* Send messages. */
   for (i=0; i<5; i++) {
           write(data_socket, &commands[i], 1);
	   clock_gettime(CLOCK_REALTIME, &rtclk_time);
	   sprintf(buf,"[sec=%ld, nanosec=%ld] CLIENT SENDING: %d\n",\
			   rtclk_time.tv_sec, rtclk_time.tv_nsec, commands[i]);
	   fputs(buf, fp);
//           printf("\nSending: %d", commands[i]);
   }

   for (i=0; i<5; i++) {
	   write(data_socket, txClient[i], sizeof(txClient[i]));
	   clock_gettime(CLOCK_REALTIME, &rtclk_time);
           sprintf(buf,"[sec=%ld, nanosec=%ld] CLIENT SENDING: %s\n",\
                           rtclk_time.tv_sec, rtclk_time.tv_nsec, txClient[i]);
           fputs(buf, fp);
//	   printf("\nSending: %s", txClient[i]);
   }

   fflush(fp);

   /* Request result. */
   strcpy (buffer, "END");
   write(data_socket, buffer, strlen(buffer) + 1);

   /* Receive results. */
   for(i=0;;i++) {
	   /* Wait for next data packet. */
	   if (i<5) {
		   read(data_socket, &result, 1);
	           clock_gettime(CLOCK_REALTIME, &rtclk_time);
        	   sprintf(buf,"[sec=%ld, nanosec=%ld] CLIENT RECEIVED: %d\n",\
                	           rtclk_time.tv_sec, rtclk_time.tv_nsec, result);
		   fputs(buf, fp);
//		   printf("\nReceived: %d", result);
	   }
	   else {
		   read(data_socket, buffer, STR_MAX_LEN);
//		   printf("\nReceived: %s", buffer);
		   if (!strncmp(buffer, "END", STR_MAX_LEN))
			   break;
		   else {
	                   clock_gettime(CLOCK_REALTIME, &rtclk_time);
        	           sprintf(buf,"[sec=%ld, nanosec=%ld] CLIENT RECEIVED: %s\n",\
                	                   rtclk_time.tv_sec, rtclk_time.tv_nsec, buffer);
			   fputs(buf, fp);
		   }
	   }
   }
   fflush(fp);

   while(!terminateSignal);
   clock_gettime(CLOCK_REALTIME, &rtclk_time);
   sprintf(buf,"[sec=%ld, nanosec=%ld] Terminating with SIGINIT signal\n",\
		   rtclk_time.tv_sec, rtclk_time.tv_nsec);
   fputs(buf,fp);
   fflush(fp);

   fclose(fp);
   /* Close socket. */
   close(data_socket);

   exit(EXIT_SUCCESS);
}
