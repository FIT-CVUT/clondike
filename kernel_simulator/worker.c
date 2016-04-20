#include "process_manager.h"
#include "worker.h"

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#define LOG_FILE "/tmp/measurement.log"

char *str2md5(const char *str, int length) {
    int n;
    MD5_CTX c;
    unsigned char digest[16];
    char *out = (char*)malloc(33);

    MD5_Init(&c);

    while (length > 0) {
        if (length > 512) {
            MD5_Update(&c, str, 512);
        } else {
            MD5_Update(&c, str, length);
        }
        length -= 512;
        str += 512;
    }

    MD5_Final(digest, &c);

    for (n = 0; n < 16; ++n) {
        snprintf(&(out[n*2]), 16*2, "%02x", (unsigned int)digest[n]);
    }

    return out;
}


static int get_hash_name(const char * name, int range_min, int range_max){
    printf("%s/n", name);
    int acc = 0;
    int i;
    for(i = 0; ; i++){
        if(name[i] == '\0')
            break;

        acc += name[i] * (range_max - range_min);
    }

    
    return (acc % (range_max - range_min)) + range_min;
}

static void work_find_prime_number(struct mig_process * p){
   
    int n = get_hash_name(p->input_line, 5000, 15000);

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

static void write_to_file(time_t t, int hashes){
    FILE * f;
    f = fopen(LOG_FILE, "a");
    if (f == NULL){
        fprintf(stderr, "cannot open file %s\n", LOG_FILE);
        fprintf(stderr, "time:%d hashes:%d\n", hashes);
        return;
    }
    fprintf(f, "%ld %d\n", t, hashes);
    fclose(f);
}

void * work(void * thread_attr){

    struct mig_process * p = thread_attr;

    //work_find_prime_number(p);
    int n = get_hash_name(p->input_line, 5000, 15000);

    time_t t1, t2;
    char * md5_hash;
    int hashes = 0;

    int i;
    for(i = 0; i < n; i++){
        t1 = time(NULL);
        md5_hash = str2md5(p->input_line, strlen(p->input_line));
        t2 = time(NULL);
        if (t2 - t1 > 0){
            //write_to_file(t1, hashes);
            hashes = 0;
            t1 = t2;
        }
        else{
            //I am still in the same second
            hashes++;
        }
        free(md5_hash);
    }
    if (hashes > 0){
//        write_to_file(t1, hashes);
    }    

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


/*
int main(){

    char * str = {"g++ -o clondike_kernel_simulator -lnl-genl-3 -lnl-3 -lpthread -lcrypto clondike_kernel_simulator.o ctlfs.o ctlfs_file_helper.o message_helper.o message_task_fork.o response_handlers.o message_task_exit.o message_npm_check.o message_node_connected.o message_node_disconnected.o message_immigration_request.o message_immigration_confirmed.o message_emigration_failed.o pen_watcher.o kkc.o kkc_messages.o process_manager.o fifo_reader.o netlink_message.o pid_manager.o worker.o kkc_socket_manager.o message_generic_user_message.ofasdfffffffffffffffffffffffffffffffffffffffffffffffffffffff"};
    int i;
    char hash[SHA512_DIGEST_LENGTH];
//    for(i = 30; i < strlen(str) + 10000; i++){
   	printf("%s\n", str2md5(str, strlen(str)));
        //SHA512(str, strlen(str), hash);
        //printf("%s\n", hash);

  //  }

}
*/

