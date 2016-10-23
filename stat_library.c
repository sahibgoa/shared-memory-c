#include <unistd.h>
#include "stat.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "stat_server.h"

#define ERROR_SHMGET "shmget failed"
#define MAX_CLIENTS 16

/*
 * Attempts to attach to an existing shared memory segment with the specified
 * key. If this is successful, it should return a pointer to the portion of the
 * shared memory segment that this client should write to for its statistics; if
 * it is not successful (e.g., the shared segment with the desired key does not
 * exist or too many clients are already using the segment for statistics), it
 * should return NULL. Each client that wishes to use the statistics monitor
 * must call stat_init().
 */
stat_t* stat_init(key_t key) {
  int seg_id = shmget(key, sizeof(stat_t), IPC_CREAT|IPC_EXCL)
  if (seg_id != EEXIST) {
      write(STDERR, ERROR_SHMGET, strlen(ERROR_SHMGET));
      return NULL;
  } else {
    stat_t *ptr = (stat_t*) shmat(seg_id, NULL, 0);
    int i = 0;
    sem_t sem;
    sem_init(&sem, 0, 1);
    sem_wait(&sem);
    // Search through the struct array to find an empty spot for client
    while (ptr->valid != 0 && i < MAX_CLIENTS) {
      ptr++;
      i++;
    }
    // Check if all slots were full
    if (ptr->valid != 0) {
      sem_post(&sem);
      return NULL;
    } else {
      sem_post(&sem);
      return ptr;
    }
  }
}

/*
 * Removes the calling process from using the shared memory segment. If
 * successful, it returns 0; if not successful (e.g., this process, or the
 * process with this pid, did not call stat_init), it should return -1
 */
int stat_unlink(key_t key) {
  pid_t pid = getpid();
  int seg_id = shmget(key, sizeof(stat_t), IPC_CREAT|IPC_EXCL)
  if (seg_id != EEXIST) {
      write(STDERR, ERROR_SHMGET, strlen(ERROR_SHMGET));
      return NULL;
  } else {
    stat_t *ptr = (stat_t*) shmat(seg_id, NULL, 0);
    int i = 0;
    sem_t sem;
    sem_init(&sem, 0, 1);
    sem_wait(&sem);
    while ((!ptr->valid || ptr->pid != pid) && i < MAX_CLIENTS) {
      ptr++;
      i++;
    }
    if (!ptr->valid || ptr->pid != pid) {
      sem_post(&sem);
      return -1;
    } else {
      sem_post(&sem);
      ptr->valid = 0;
      return 0;
    }
}
