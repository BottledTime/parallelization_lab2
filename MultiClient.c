/* Client in the implementation of a Server-Client communication.
*                Firsta approach: via mutexes.
*
*  Description: A multithread client launches X simultaneous
*  read or write requests to this multithread server. The
*  client sends a position and a read or write identifier,
*  both randomly generated, so the server knows where to
*  read or write in the array theArray.
*
* IN:
*       Port number: port
*       Array size: n
*
* Compile as:
*
* Usage:
*
* Students:
*        Breno Bahia (ID: 1449808)
*        Derek
*/

// include headers
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "timer.h"

// Define and declare global variables
#define REQUEST_THREADS 10000
#define STR_LEN 1000
int port, n;

// Functions prototypes
void* ConnectServer( void* ); // Thread function

/*--------------------------------------------------------------------*/
                       /* Main Function */
int main(int argc, char* argv[]){

   // Declare local variables
   long thread;
	pthread_t thread_handles[REQUEST_THREADS]; 
	double start, finish;
	
		// Safe-guard for more than 2 arguments
	if (argc > 3){
   	printf("Too many input arguments. Supply only PORT and N.\n\n");
   	printf("Terminating ... \n\n");
   	exit(0);
	}	
	
	// Get port number from command line
	port = strtol(argv[1], NULL, 10);
	// Get array size from command line
	n = strtol(argv[2], NULL, 10);
		
   // Start multiclient requests
   GET_TIME(start);
   
   for (thread=0; thread<REQUEST_THREADS; thread++){
      pthread_create(&thread_handles[thread], NULL, ConnectServer, (void*) thread);
   } /* end for */

   // Join threads
	for (thread = 0; thread < REQUEST_THREADS; thread++){ 
	pthread_join(thread_handles[thread], NULL); 
   } /* end for */
   
	GET_TIME(finish);
   
 	printf("The elapsed time is %e seconds\n", finish-start);
   
   // Terminate successfully
   return 0;
   
} /* end main */
   
void *ConnectServer(void* rank){   

   // Declare variables
	struct sockaddr_in sock_var;
	long my_rank = (long) rank;
	int clientFileDescriptor = socket(AF_INET,SOCK_STREAM,0);
	char str_ser[STR_LEN];	

   // Network stuff
	sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
	sock_var.sin_port=port;
	sock_var.sin_family=AF_INET;

   // Try to connect to server
	if(connect(clientFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0){
	
	   // Comment when testing
		printf("Connected to server %d\n\n",clientFileDescriptor);

		// Send to thread rank to server
		write(clientFileDescriptor,&my_rank,sizeof(my_rank));

      // Read from server
		read(clientFileDescriptor,str_ser,STR_LEN);
		
	   // Comment when testing
		printf("String from Server: %s\n\n",str_ser);
		
		// Close connection
		close(clientFileDescriptor);
	}
	else{
		printf("socket creation failed\n");
	}
	return 0;
} /* end connect_to_server */
