#include "stat.h"
#include <sys/ipc.h>
#include <sys/shm.h>


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
  if ((seg_id = shmget(key, size, IPC_CREAT)) < 0) {
    write(STDERR, ERROR_SHMGET, strlen(ERROR_SHMGET));
  }
  char *ptr = shmat(seg_id, NULL, 0);
}

int stat_unlink(key_t key) {

}
