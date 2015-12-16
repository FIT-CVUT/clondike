#ifndef MIGRATED_HOME_MSG_H
#define MIGRATED_HOME_MSG_H

#include <linux/types.h>

/**
 * Called, when a task is migrated home (called on ccn)
 *
 * @param pid Pid of the task that has returned
 * @param name Name of a new task that returned home
 * @param jif Jiffies as identifier of a new task that returned home
 * @return 0 on success, error code otherwise
 */
int migrated_home(pid_t pid, const char* name, unsigned long jif);

#endif
