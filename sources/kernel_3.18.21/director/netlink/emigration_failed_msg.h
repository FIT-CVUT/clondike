#ifndef EMIGRATION_FAILED_MSG_H
#define EMIGRATION_FAILED_MSG_H

#include <linux/types.h>

/**
 * Called, when an emigration request for task has failed
 *
 * @param pid Pid of the task that failed to emigrate
 * @param name Name of the task that failed to emigrate
 * @param jif Jiffies as identifier of the task that failed to emigrate
 * @return 0 on success, error code otherwise
 */
int emigration_failed(pid_t pid, const char* name,	unsigned long jif);

#endif
