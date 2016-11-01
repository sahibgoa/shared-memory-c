#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include "stats.h"

#define ERROR_SHMGET_INIT "shmget failed in stats_init\n"
#define ERROR_SHMGET_UNLILNK "shmget failed in stats_unlink\n"
#define MAX_CLIENTS 16
#define STDOUT 1
#define STDERR 2

sem_t *mutex;

stats_t* stats_init(key_t key) {
  if ((mutex = sem_open("sahib-se", 0)) == SEM_FAILED) {
    perror("sem_open failed in stats_init\n");
    return NULL;
  }
  int seg_id = shmget(key, sizeof(stats_t), 0666);
  if (seg_id == -1) {  // call fails when segment exists
      write(STDERR, ERROR_SHMGET_INIT, strlen(ERROR_SHMGET_INIT));
      return NULL;
  } else {
    seg_id = shmget(key, sizeof(stats_t), 0);
    stats_t *ptr = (stats_t*) shmat(seg_id, NULL, 0);
    if (ptr == (stats_t*) -1) {
      return NULL;
    }
    int i = 0;
    if (mutex == SEM_FAILED) {
      perror("sem_open failed in stats_init\n");
      return NULL;
    }

    sem_wait(mutex);
    // Search through the struct array to find an empty spot for client
    while (ptr->valid != 0 && i < MAX_CLIENTS) {
      ptr++;
      i++;
    }

    // Check if all slots were full
    if (i == MAX_CLIENTS) {
      sem_post(mutex);
      return NULL;
    } else {
      sem_post(mutex);
      ptr->valid = 1;
      return ptr;
    }
  }
}

int stats_unlink(key_t key) {
  pid_t pid = getpid();
  int seg_id = shmget(key, sizeof(stats_t), 0666);
  if (seg_id == -1) {
      write(STDERR, ERROR_SHMGET_UNLILNK, strlen(ERROR_SHMGET_UNLILNK));
      return -1;
  } else {
    stats_t *ptr = (stats_t*) shmat(seg_id, NULL, 0);
    int i = 0;
    sem_wait(mutex);

    while ((!ptr->valid || ptr->pid != pid) && i < MAX_CLIENTS) {
      ptr++;
      i++;
    }
    if (!ptr->valid || ptr->pid != pid) {
      sem_post(mutex);
      return -1;
    } else {
      ptr->valid = 0;
      sem_post(mutex);
      return 0;
    }
  }
}
