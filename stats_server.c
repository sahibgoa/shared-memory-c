#include <ctype.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#include "stats_server.h"
#include "stats.h"

#define STDOUT 1
#define STDERR 2
#define ERROR_USAGE "Usage: stat_server -k [key]\n"
#define ERROR_SIGINT "Error when setting up sigaction for SIGINT\n"
#define ERROR_SHMGET "shmget failed"
#define MAX_CLIENTS 16

int seg_id, num_clients = 0;
sem_t *mutex;

// Handler for SIGINT
void sigint_handler(int signal)
{
  // Remove shared memory segment
  if (shmctl(seg_id, IPC_RMID, NULL) == -1) {
    perror("shmctl");
    exit(1);
  }

  // Unlink semaphore
  if (sem_unlink("mysemaphore") == -1) {
    perror("sem_unlink");
    exit(1);
  }

  exit(0);
}

// Prints whether the given string is a number
int is_number(char *num) {
  int i = 0;
  while (i < strlen(num)) {
    if (!isdigit(num[i])) {
      return 0;
    }
    i++;
  }
  return 1;
}

// Return the number of characters in the given integer
int num_chars_in_int(int integer) {
  int tens, num_chars = 1;
  if (integer < 0) {
    num_chars = 1;
    tens = -1;
    while (tens > integer) {
      tens *= 10;
      num_chars++;
    }
  } else {
    tens = 1;
    while (tens < integer) {
      tens *= 10;
      num_chars++;
    }
  }
  return num_chars;
}


int main(int argc, char *argv[]) {
  // Set up the SIGINT handler
  struct sigaction sigint;
  memset(&sigint,0,sizeof(sigint));
  sigint.sa_handler = sigint_handler;
  if(sigaction(SIGINT, &sigint, NULL) !=0 )
  {
    write(STDOUT, ERROR_SIGINT, strlen(ERROR_SIGINT));
    exit(0);
  }

  // Check for usage error
  if (argc != 3 || strcmp(argv[1], "-k") != 0 || !is_number(argv[2])) {
    write(STDERR, ERROR_USAGE, strlen(ERROR_USAGE));
    exit(1);
  }

  // Declaring all variables required
  key_t key = atoi(argv[2]);
  int j, i = 1;
  long pagesize = getpagesize();
  stats_t *ptr, *rd_ptr;
  char *line;

  // Create and initialize memory segment
  seg_id = shmget(key, pagesize, IPC_CREAT);
  if (seg_id < 0) {
    perror("shmget");
    exit(1);
  }

  // Attach to the shared memory segment
  ptr = (stats_t*) shmat(seg_id, NULL, 0);
  if (ptr == NULL) {
    perror("shmat");
    exit(1);
  }

  //
  if ((mutex = sem_open("mysemaphore", O_CREAT, 0644, 1)) == SEM_FAILED) {
    perror("sem_open");
    exit(1);
  }
  sem_init(mutex, 0, 1);

  // Infinite loop to read data
  while (1) {
    sleep(1);

    // Print all the client statistics
    if (sem_wait(mutex) < 0) {
        perror("sem_wait");
    }
    j = 0;
    for (rd_ptr = ptr; j < num_clients; j++, rd_ptr++) {
      if(rd_ptr->valid == 1) {
        line = (char*) malloc(sizeof(stats_t) - sizeof(int) + 6 +
                                    strlen(rd_ptr->name) + num_chars_in_int(i));
        sprintf(line, "%d %d %s %d %.2f %d\n", i, rd_ptr->pid, rd_ptr->name,
              rd_ptr->counter, rd_ptr->cpu_secs, rd_ptr->priority);
        write(STDOUT, line, strlen(line));
        free(line);
      }
    }
    if (sem_post(mutex) < 0) {
        perror("sem_post");
    }
    write(STDOUT, "\n", 1);
    i++;
  }

  return 0;
}
