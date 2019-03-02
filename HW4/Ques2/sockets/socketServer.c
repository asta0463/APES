#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
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
   struct sockaddr_un name;
   int i, connection_socket, data_socket;
   uint8_t result;
   char buffer[STR_MAX_LEN], buf[100];

   FILE *fp;
   struct timespec rtclk_time = {0, 0};

   //Messages to be transmitted by the server
   char txServer[5][STR_MAX_LEN]= {"Fabinho", "Robertson", "Alexander Arnold", "Lovren",\
		"Wijnaldum"};
   uint8_t commands[5]= {1, 0, 1, 1, 0}; //ON/OFF signals

   struct sigaction sa;
   sigemptyset (&sa.sa_mask);
   sa.sa_handler=&sigHandler;
   sa.sa_flags=0;
   sigaction(SIGINT, &sa, NULL);


   /* In case the program exited inadvertently on the last run,
    * remove the socket.*/
   unlink(SOCKET_NAME);

   /* Create local socket. */
   connection_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);

   /*Clear the whole structure. */
   memset(&name, 0, sizeof(struct sockaddr_un));

   /* Bind socket to socket name. */
   name.sun_family = AF_UNIX;
   strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

   bind(connection_socket, (const struct sockaddr *) &name,
			  sizeof(struct sockaddr_un));

   /*Prepare for accepting connections. The backlog size is set to 20.*/
   listen(connection_socket, 20);


   fp=fopen("output2.txt", "a");

   sprintf(buf,"PID %d;  IPC method: Sockets  Resource: localsocket\n",getpid());
   fputs(buf, fp);
   fflush(fp);

   /* Main loop for handling connections. */
   while(1) {
	   /* Wait for incoming connection. */
	   printf("1");
	   data_socket = accept(connection_socket, NULL, NULL);
	   printf("2");

	   for(i=0;;i++) {
		   /* Wait for next data packet. */
		   if (i<5){
			   read(data_socket, &result, 1);
	                   clock_gettime(CLOCK_REALTIME, &rtclk_time);
        	           sprintf(buf,"[sec=%ld, nanosec=%ld] SERVER RECEIVED: %d\n",\
                	                   rtclk_time.tv_sec, rtclk_time.tv_nsec, result);
			   fputs(buf, fp);
//			   printf("\nReceived: %d", result);
		   }
		   else {
			   read(data_socket, buffer, STR_MAX_LEN);
//			   printf("\nReceived: %s", buffer);
			   if (!strncmp(buffer, "END", STR_MAX_LEN))
				   break;
			   else {
				   clock_gettime(CLOCK_REALTIME, &rtclk_time);
				   sprintf(buf,"[sec=%ld, nanosec=%ld] SERVER RECEIVED: %s\n",\
						   rtclk_time.tv_sec, rtclk_time.tv_nsec, buffer);
				   fputs(buf, fp);
			   }
		   }
	   }

	   fflush(fp);

     	   for (i=0; i<5; i++) {
		   write(data_socket, &commands[i], 1);
	           clock_gettime(CLOCK_REALTIME, &rtclk_time);
        	   sprintf(buf,"[sec=%ld, nanosec=%ld] SERVER SENDING: %d\n",\
                	           rtclk_time.tv_sec, rtclk_time.tv_nsec, commands[i]);
		   fputs(buf, fp);
//		   printf("\nSending: %d", commands[i]);
	   }
	   for (i=0; i<5; i++) {
		   write(data_socket, txServer[i], sizeof(txServer[i]));
	           clock_gettime(CLOCK_REALTIME, &rtclk_time);
        	   sprintf(buf,"[sec=%ld, nanosec=%ld] SERVER SENDING: %s\n",\
                	           rtclk_time.tv_sec, rtclk_time.tv_nsec, txServer[i]);
		   fputs(buf, fp);
//		   printf("\nSending: %s", txServer[i]);
	   }

	   fflush(fp);
	   fclose(fp);
	   strcpy (buffer, "END");
	   write(data_socket, buffer, sizeof(buffer));
//	   printf("\nSending: %s", buffer);


	   /* Close socket. */

	   close(data_socket);

   }

   close(connection_socket);

   /* Unlink the socket. */

   unlink(SOCKET_NAME);

   exit(EXIT_SUCCESS);
}
