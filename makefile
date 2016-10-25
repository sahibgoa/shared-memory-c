CC=gcc
CFLAGS=-Wall -g -Werror

all:
	export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
	$(CC) -c -fpic stats_library.c $(CFLAGS)
	$(CC) -shared -o libstats.so stats_library.o
	$(CC) -o stats_server stats_server.c $(CFLAGS) -lstats -L. -lpthread
	$(CC) -o stats_client stats_client.c $(CFLAGS) -lstats -L. -lpthread

clean:
	$(RM) stats_server
	$(RM) stats_client
	$(RM) libstats.so
	$(RM) *.o
