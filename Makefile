CC=gcc
CFLAGS=-g -Wall -lpthread

main: MultiClient.c mtx_MultiServer.c mtx_foreach_MultiServer.c rw_foreach_MultiServer.c rw_MultiServer.c
	$(CC) -o client MultiClient.c $(CFLAGS)
	$(CC) -o mtx_server mtx_MultiServer.c $(CFLAGS)
	$(CC) -o mtx_foreach_server mtx_foreach_MultiServer.c $(CFLAGS)
	$(CC) -o rw_foreach_server rw_foreach_MultiServer.c $(CFLAGS)
	$(CC) -o rw_server rw_MultiServer.c $(CFLAGS)
