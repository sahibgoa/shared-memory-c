#include "stat.h"
#include <sys/ipc.h>
#include <sys/shm.h>

#define ERROR_USAGE "Usage: stats_client -k key -p priority -s sleeptime_ns -c \
cputime_ns"

void usage(char *) {
  write(STDERR, ERROR_USAGE, strlen(ERROR_USAGE));
  exit(1);
}

int main(int argc, char *argv[]) {
  key_t key = 1;
  int priority = 1, sleeptime_ns = 1000, cputime_ns = 1000;
  int c;
  opterr = 0;
  while ((c = getopt(argc, argv, "k:p:s:c")) != -1) {
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
      cputime_ns = strdup(optarg);
      break;
    default:
      usage(argv[0]);
    }
  }
  return 0;
}
