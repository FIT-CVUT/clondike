#include "process_manager.h"
#include "worker.h"

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

void * work(void * thread_attr){

    struct mig_process * p = thread_attr;

    srand(time(NULL));
    int t = rand();
    t = t % MAX_WORKING_TIME;
    printf("process %d will work %d seconds.\n", p->pid, t);

    sleep(t);

    p->return_code = 0;
    p->migration_state = MIG_PROCESS_END;
    
    pthread_exit(NULL);
}

int fork_and_work(struct mig_process * p){
   
    pthread_t thread;

    int ret;

    ret = pthread_create(&thread, NULL, work, p);
    
    if(ret){
        printf("cannot fork worker process\n");
        return -1;
    }

    return 0;
}




