#ifndef WORKER_H
#define WORKER_H

#define MAX_WORKING_TIME    10

#ifdef __cplusplus
extern "C" {
#endif



int fork_and_work(struct mig_process * p);


#ifdef __cplusplus
}
#endif

#endif
