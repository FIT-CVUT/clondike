#include "process_manager.h"
#include "worker.h"

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>


static void work_find_prime_number(){
    srand(time(NULL));
    int n = rand();

    //range is <60000 - 120000>
    n = n % 6000;
    n += 6000;

    int count = 0;
    long a = 2;

    while (count < n){
        long b = 2;
        int is_prime = 1;
        while(b*b <= a){
            if (a % b == 0){
                is_prime = 0;
                break;
            }
            b++;
        }
        if (is_prime == 1)
            count++;

        a++;
    }

}

static void work_time(struct mig_process * p){
    srand(time(NULL));
    int t = rand();
    t = t % MAX_WORKING_TIME;
    printf("process %d will work %d seconds.\n", p->pid, t);

    sleep(t);
}

void * work(void * thread_attr){

    struct mig_process * p = thread_attr;

    work_find_prime_number();

    p->return_code = 0;
    p->migration_state = MIG_PROCESS_END;
    
    printf("process %d terminated.\n", p->pid);
    
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




