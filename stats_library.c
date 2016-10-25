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

#define ERROR_SHMGET "shmget failed"
#define MAX_CLIENTS 16
#define STDOUT 1
#define STDERR 2

stats_t* stats_init(key_t key) {
  sem_t *mutex;
  int seg_id = shmget(key, sizeof(stats_t), IPC_CREAT|IPC_EXCL);
  if (seg_id == -1) {
      write(STDERR, ERROR_SHMGET, strlen(ERROR_SHMGET));
      return NULL;
  } else {
    // seg_id = shmget(key, sizeof(stats_t), IPC_CREAT|SHM_W);
    stats_t *ptr = (stats_t*) shmat(seg_id, NULL, 0);
    int i = 0;

    if ((mutex = sem_open("mysemaphore", O_CREAT, 0644, 1)) == SEM_FAILED) {
      perror("sem_open");
      exit(1);
    }

    sem_wait(mutex);
    // Search through the struct array to find an empty spot for client
    while (ptr->valid != 0 && i < MAX_CLIENTS) {
      ptr++;
      i++;
    }
    // Check if all slots were full
    if (ptr->valid != 0) {
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
  sem_t *mutex;
  pid_t pid = getpid();
  int seg_id = shmget(key, sizeof(stats_t), IPC_CREAT|IPC_EXCL);
  if (seg_id == -1) {
      write(STDERR, ERROR_SHMGET, strlen(ERROR_SHMGET));
      return -1;
  } else {
    // seg_id = shmget(key, sizeof(stats_t), IPC_CREAT);
    stats_t *ptr = (stats_t*) shmat(seg_id, NULL, 0);
    int i = 0;

    if ((mutex = sem_open("mysemaphore", O_CREAT, 0644, 1)) == SEM_FAILED) {
      perror("sem_open");
      exit(1);
    }
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
