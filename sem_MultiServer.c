/* Server in the implementation of a Server-Client communication.
*                Second approach: via busy-wait.
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
#include <semaphore.h>

// Define and declare global variables
#define REQUEST_THREADS 1000
#define STR_LEN 1000
int port, n, counter, writes;
int* seed;
char** theArray;
sem_t count_sem;
sem_t barrier_sem;

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
	long thread;
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

	// Initialize semaphore
	sem_init(&barrier_sem,0,0); // Initially blocked
	sem_init(&count_sem,0,1); // Initially unlocked

	// Allocate memory for theArray using malloc and fill the array
	theArray = (char **)malloc(n*sizeof(char *));
	for (i = 0; i < n; i++){
		theArray[i]=(char *)malloc(STR_LEN*sizeof(char));
		sprintf(theArray[i],"String %d: the initial value\n", i);
		printf("\nArray %d created as %s\n\n", i, theArray[i]);
	} /* end for */

	// Network stuff
	sock_var.sin_addr.s_addr=inet_addr("127.0.0.1");
	sock_var.sin_port=port;
	sock_var.sin_family=AF_INET;

	if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0){
		printf("\nsocket has been created \n");
		listen(serverFileDescriptor,2000);

		// sleep(.5);

		while(1){        //loop infinity

			for (thread=0; thread<REQUEST_THREADS; thread++){  //can support REQUEST_THREADS clients at a time

				// Accept connection from client
				clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
				printf("\nConnected to client %d\n",clientFileDescriptor);

				// Handle multiple clients with pthreads
				pthread_create(&t[thread],NULL,ServerEcho,(void *)clientFileDescriptor);
			} /* end for */
		} /* end while */

		// close connection
		close(serverFileDescriptor);

		// destroy semaphores
		sem_destroy(&barrier_sem);
		sem_destroy(&count_sem);

	} /* end if */
	else{
		printf("nsocket creation failed\n\n");
	} /* end else */

	// Terminate successfully
	return 0;
}
/* end main */
/*--------------------------------------------------------------------*/
/* Thread Function */
void* ServerEcho( void* my_rank ){

	// Declare variables
	int clientFileDescriptor = (int) my_rank;
	int clnt_rank;
	int pos, randRW, i;

	// Read request from client
	//printf("\nreading from client %d \n",clientFileDescriptor);
	read(clientFileDescriptor,&clnt_rank,sizeof(clnt_rank));

	// Identify client rank:
	pos=rand_r( &clnt_rank ) % n;

	// Identify request (read or write) from client:
	randRW=rand_r( &clnt_rank ) % 100;
	/* randRW goes from 0 to 99. So we have 5% chances
	of this number being larger than or equal to 94.
	Careful not to divide by 5: the numbers will range
	from 0 to 4, and a number equal to 4 has 20%
	chance of happening. */

	// Semaphore barrier:

	// Lock all threads (this locks them as each thread calls sem_wait()
	sem_wait( &count_sem );

	/* If this is the last thread, resent count to 0 and count_sem to 1
	(both can now be reused in another barrier). Unlock every operating
	thread with sem_post (barrier_sem should not be reused due to race cond.)
	*/
	if ( counter == REQUEST_THREADS - 1){
		// Reset
		counter=0;
		sem_post(&count_sem);

		// Unlock waiting threads
		for (i = 0; i < REQUEST_THREADS; i++){
			sem_post(&barrier_sem);
		} /* end for */
	} /* end if */

	/* If this is not the last thread, increase counter, unlock count_sem
	so next thread can go in the barrier (otherwise sem_wait in line 150
	will lock the threads forever), and lock the thread in the barrier */
	else{
		counter++;
		sem_post(&count_sem);
		sem_wait(&barrier_sem);
	}


	// end semaphore barrier. Go to action

	// 5% are write operations, others are reads.
	if (randRW >= 94){
		// Modify string if the above is true
		sprintf(theArray[pos], "String %d has been modified by a write request\n", pos);
		writes++;
		printf("Total of writes is %d \n", writes);
	} /* end if */

	// Send str back to client
	write(clientFileDescriptor,theArray[pos],STR_LEN);
	printf("\nechoing back to client \n\n");

	// Close connection
	close(clientFileDescriptor);

	// return something to avoid warnings
	return 0;
} /* end ServerEcho */
