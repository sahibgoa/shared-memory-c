#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "stats.h"

#define ERROR_USAGE "Usage: stats_client -k key -p priority -s sleeptime_n "\
                    "-c cputime_ns\n"
#define ERROR_SIGINT "Error when setting up sigaction for SIGINT\n"
#define BILLION  1000000000L
#define STDOUT 1
#define STDERR 2

stats_t* stats_init(key_t key);
int stats_unlink(key_t key);

key_t key;
sem_t *mutex;

// handler for SIGINT
void sigint_handler(int signal) {
  write(STDOUT, "\n", 1);

  stats_unlink(key);
  if (sem_close(mutex) == -1) {
    perror("sem_close failed in client\n");
    exit(1);
  }
  exit(0);
}

// Print usage error
void usage(char *name) {
  write(STDERR, ERROR_USAGE, strlen(ERROR_USAGE));
  exit(1);
}

int main(int argc, char *argv[]) {
  // Set up the SIGINT handler
  struct sigaction sigint;
  memset(&sigint, 0, sizeof(sigint));
  sigint.sa_handler = sigint_handler;
  if (sigaction(SIGINT, &sigint, NULL) != 0) {
    write(STDOUT, ERROR_SIGINT, strlen(ERROR_SIGINT));
    exit(0);
  }

  key = 1;
  int c, priority = 1;
  long sleeptime_ns = 1000, cputime_ns = 1000;
  opterr = 0;
  while ((c = getopt(argc, argv, "k:p:s:c:")) != -1) {
    switch (c) {
    case 'k':
      key = atoi(optarg);
      break;
    case 'p':
      priority = atoi(optarg);
      break;
    case 's':
      sleeptime_ns = atoi(optarg);
      break;
    case 'c':
      cputime_ns = atoi(optarg);
      break;
    default:
      usage(argv[0]);
    }
  }
  if ((mutex = sem_open("sahib-se", 0)) == SEM_FAILED) {
    perror("sem_open failed in stats_init\n");
    exit(1);
  }

  stats_t *ptr = stats_init(key);

  if (ptr == NULL) {  // server is not running?
    exit(1);
  }
  ptr->pid = getpid();
  ptr->counter = 0;
  ptr->priority = priority;
  if (setpriority(PRIO_PROCESS, 0, ptr->priority) == -1) {
    perror("setpriority failed in client\n");
    // exit(1);
  }
  ptr->cpu_secs = 0.0;

  // Truncate name
  if (strlen(argv[0]) > 15) {
    argv[0][15] = '\0';
  }
  strcpy(ptr->name, argv[0]);

  struct timespec tim_begin, tim_init, tim_final, tim_nsleep, tim;
  double diff;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tim_begin);

  tim_nsleep.tv_sec = sleeptime_ns / BILLION;
  tim_nsleep.tv_nsec = sleeptime_ns % BILLION;

  while (1) {
    if (nanosleep(&tim_nsleep, &tim) < 0) {
      perror("nanosleep failed in client\n");
    }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tim_init);
    do {
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tim_final);
      diff = (tim_final.tv_nsec - tim_init.tv_nsec) +
        (tim_final.tv_sec - tim_init.tv_sec) * BILLION;
    } while (diff < cputime_ns);

    ptr->cpu_secs =
      ((double)tim_final.tv_sec + (double)tim_final.tv_nsec/BILLION) -
      ((double)tim_begin.tv_sec + (double)tim_begin.tv_nsec/BILLION);

    ptr->priority = getpriority(PRIO_PROCESS, 0);
    ptr->counter++;
  }

  return 0;
}
