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

// Define struct for rwlock
typedef struct {
	int readers;
	int writer;
	pthread_cond_t readers_proceed;
	pthread_cond_t writer_proceed;
	int pending_writers;
	pthread_mutex_t read_write_lock;
} mylib_rwlock_t;

// Define and declare global variables
#define REQUEST_THREADS 1000
#define STR_LEN 1000
int port, n, writes;
int* seed;
char** theArray;
pthread_mutex_t mutex;
mylib_rwlock_t* rwlocks;

// Function prototypes
void* ServerEcho( void* ); // Thread function

// rwlock from eClass

void mylib_rwlock_init (mylib_rwlock_t *l) {
	l -> readers = l -> writer = l -> pending_writers = 0;
	pthread_mutex_init(&(l -> read_write_lock), NULL);
	pthread_cond_init(&(l -> readers_proceed), NULL);
	pthread_cond_init(&(l -> writer_proceed), NULL);
}

void mylib_rwlock_rlock(mylib_rwlock_t *l) {
	/* if there is a write lock or pending writers, perform condition wait, else increment count of readers and grant read lock */

	pthread_mutex_lock(&(l -> read_write_lock));
	while ((l -> pending_writers > 0) || (l -> writer > 0))
	pthread_cond_wait(&(l -> readers_proceed),
	&(l -> read_write_lock));
	l -> readers ++;
	pthread_mutex_unlock(&(l -> read_write_lock));
}

void mylib_rwlock_wlock(mylib_rwlock_t *l) {
	/* if there are readers or writers, increment pending writers count and wait. On being woken, decrement pending writers count and increment writer count */

	pthread_mutex_lock(&(l -> read_write_lock));
	while ((l -> writer > 0) || (l -> readers > 0)) {
		l -> pending_writers ++;
		pthread_cond_wait(&(l -> writer_proceed),
		&(l -> read_write_lock));
		l -> pending_writers --;
	}
	l -> writer ++;
	pthread_mutex_unlock(&(l -> read_write_lock));
}

void mylib_rwlock_unlock(mylib_rwlock_t *l) {
	/* if there is a write lock then unlock, else if there are read locks, decrement count of read locks. If the count is 0 and there is a pending writer, let it through, else if there are pending readers, let them all go through */

	pthread_mutex_lock(&(l -> read_write_lock));
	if (l -> writer > 0)
	l -> writer = 0;
	else if (l -> readers > 0)
	l -> readers --;
	pthread_mutex_unlock(&(l -> read_write_lock));

	if (l -> readers > 0)
	pthread_cond_broadcast(&(l -> readers_proceed));
	else if ((l -> readers == 0) && (l -> pending_writers > 0))
	pthread_cond_signal(&(l -> writer_proceed));
	else if ((l -> readers == 0) && (l -> pending_writers == 0))
	pthread_cond_broadcast(&(l -> readers_proceed));
}

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

	// Initialize rwlocks
	rwlocks = (mylib_rwlock_t *)malloc(n*sizeof(mylib_rwlock_t));
	for (i = 0; i < n; i++) {
		mylib_rwlock_init(&rwlocks[i]);
	}

	if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0){
		// printf("\nsocket has been created \n");
		listen(serverFileDescriptor,2000);

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
		free(t);

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
	int clientFileDescriptor = (intptr_t) my_rank;
	unsigned int sentValues[2];

	// Read request from client
	read(clientFileDescriptor,sentValues,sizeof(sentValues));
	unsigned int toWrite = sentValues[1];
	unsigned int pos = sentValues[0];

	/* printf("\nreading from client:%d \n",serv_rec); */

	GET_TIME(start);
	// 5% are write operations, others are reads.
	if (toWrite){
		/* The above is true this is a write request.
		Thus, unlock read  (line 205) and lock write (below) */

		// mylib_rwlock_unlock(&rwlock);
		mylib_rwlock_wlock(&rwlocks[pos]);

		// Modify string
		sprintf(theArray[pos], "String %d has been modified by a write request\n", pos);
		writes++;
		// printf("Total of writes is %d \n", writes);

		// Unlock mutex
		mylib_rwlock_unlock(&rwlocks[pos]);

	} /* end if */

	// Send str back to client
	mylib_rwlock_rlock(&rwlocks[pos]);

	char* myData = theArray[pos];

	mylib_rwlock_unlock(&rwlocks[pos]);
	GET_TIME(finish);
	totalTime += (finish - start);

	write(clientFileDescriptor,myData,STR_LEN);
	// printf("\nechoing back to client \n\n");

	// Close connection
	close(clientFileDescriptor);

	// Exit threads
	pthread_exit(0);

	// return something to avoid warnings
	return 0;
} /* ServerEcho */
