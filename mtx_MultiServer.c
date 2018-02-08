/* Server in the implementation of a Server-Client communication.
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
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "timer.h"

double totalTime;

// Define and declare global variables
#define REQUEST_THREADS 1000
#define STR_LEN 1000
int port, n, writes;
int* seed;
char** theArray;
pthread_mutex_t mutex;

// Function prototypes
void* ServerEcho( void* ); // Thread function

/*--------------------------------------------------------------------*/
/* Main Function */
int main(int argc, char* argv[]){

	// Declare local variables
	struct sockaddr_in sock_var;
	int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
	int clientFileDescriptor;
	int i;
	pthread_t t[REQUEST_THREADS];


	// Safe-guard for more than 2 arguments
	if (argc > 3){
		printf("Too many input arguments. Supply only PORT and N.\n\n");
		printf("Terminating ... \n\n");
		exit(0);
	}	/* end for */

	// Get port number from command line
	port = strtol(argv[1], NULL, 10);
	// Get array size from command line
	n = strtol(argv[2], NULL, 10);

	// Allocate memory for theArray using malloc and fill the array
	theArray = (char **)malloc(n*sizeof(char *));
	for (i = 0; i < n; i++){
		theArray[i]=(char *)malloc(STR_LEN*sizeof(char));
		sprintf(theArray[i],"String %d: the initial value\n", i);
		// printf("Array %d created as %s\n\n", i, theArray[i]);
	} /* end for */

	// Network stuff
	sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
	sock_var.sin_port=port;
	sock_var.sin_family=AF_INET;

	// Initialize mutex
	pthread_mutex_init(&mutex, NULL);

	if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0){
		// printf("\nsocket has been created \n");
		listen(serverFileDescriptor,2000);

		// This sleep was causing a warning, since it takes an integer argument,
		// not a float. It converts it to 0, so this does nothing.
		// sleep(.5);

		while(1){        //loop infinity

			totalTime = 0;

			for(i=0; i<REQUEST_THREADS; i++){  //can support REQUEST_THREADS clients at a time

				// Accept connection from client
				clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
				// printf("\nConnected to client %d\n",clientFileDescriptor);

				// Handle multiple clients with pthreads
				pthread_create(&t[i],NULL,ServerEcho,(void *) (intptr_t) clientFileDescriptor);
			} /* end for */

			// Join threads
			for (i = 0; i < REQUEST_THREADS; i++){
				pthread_join(t[i], NULL);
			} /* end for */
			printf("%f\n", totalTime);

		} /* end while */

		// close connection
		close(serverFileDescriptor);

		// destroy mutex
		pthread_mutex_destroy(&mutex);

	} /* end if */
	else{
		printf("nsocket creation failed\n\n");
	} /* end else */

	// Terminate successfully
	return 0;
} /* end main */

void* ServerEcho( void* my_rank ){

	// Declare variables
	double start;
	double finish;
	int clientFileDescriptor = (intptr_t)  my_rank;
	unsigned int sentValues[2];

	// Read request from client
	read(clientFileDescriptor,sentValues,sizeof(sentValues));
	unsigned int toWrite = sentValues[1];
	unsigned int pos = sentValues[0];

	/* printf("\nreading from client:%d \n",serv_rec); */

	GET_TIME(start);

	// Lock mutex
	pthread_mutex_lock(&mutex);

	// 5% are write operations, others are reads.
	if (toWrite){
		// Modify string if the above is true
		sprintf(theArray[pos], "String %d has been modified by a write request\n", pos);
		// writes++;
		// printf("Total of writes is %d \n", writes);
	} /* end if */

	// Unlock mutex
	pthread_mutex_unlock(&mutex);

	// Send str back to client
	write(clientFileDescriptor,theArray[pos],STR_LEN);
	GET_TIME(finish);
	totalTime += (finish - start);
	// printf("\nechoing back to client \n\n");

	// Close connection
	close(clientFileDescriptor);

	// return something to avoid warnings
	return 0;
}
