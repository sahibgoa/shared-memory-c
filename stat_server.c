#include <sys/ipc.h>
#include <sys/shm.h>

#define STDERR 0
#define STDOUT 1
#define ERROR_SHMGET "Could not create shared memory"

int main(int argc, char *argv[]) {

  key_t key;
  char *s, *shm;
  int seg_id;
  
  if ((seg_id = shmget(key, size, IPC_CREAT)) < 0) {
    write(STDERR, ERROR_SHMGET, strlen(ERROR_SHMGET));
  }

  char *ptr = shmat(seg_id, NULL, 0);

}
