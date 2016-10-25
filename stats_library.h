#ifndef stats_library_h__
#define stats_library_h__

/*
 * Attempts to attach to an existing shared memory segment with the specified
 * key. If this is successful, it should return a pointer to the portion of the
 * shared memory segment that this client should write to for its statistics; if
 * it is not successful (e.g., the shared segment with the desired key does not
 * exist or too many clients are already using the segment for statistics), it
 * should return NULL. Each client that wishes to use the statistics monitor
 * must call stat_init().
 */
extern stats_t* stats_init(key_t key);

/*
 * Removes the calling process from using the shared memory segment. If
 * successful, it returns 0; if not successful (e.g., this process, or the
 * process with this pid, did not call stat_init), it should return -1
 */
extern int stats_unlink(key_t key);


#endif  // foo_h__
