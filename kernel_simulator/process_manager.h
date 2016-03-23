#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum migration_state{
    MIG_PROCESS_PREPARED,
    MIG_PROCESS_NEW,
    MIG_PROCESS_REQUEST,
    MIG_PROCESS_CONFIRMED,
    MIG_PROCESS_CONFIRMED_SEND,
    MIG_PROCESS_DENIED,
    MIG_PROCESS_BEGIN,
    MIG_PROCESS_END,
    MIG_PROCESS_WORKING,
    MIG_PROCESS_WORKING_LOCALY,
    MIG_PROCESS_CLEAN
};


struct mig_process {
    int pid;
    int remote_pid; //actually, it means local pid for fake working process
    char name[100];
    int uid;
    int peer_index;
    int migration_state;
    int return_code; //used for return code of program and decision about acceptation of migration
    uint64_t jiffies;
    unsigned int sequence_number;
};

int emig_process_migrate(unsigned int sequence_number, int peer_index);

int emig_process_put(int pid, const char * name, int uid, unsigned int seq);

int emig_process_migration_confirmed(int pid, int decision);

int emig_process_done(int pid, int return_code);

int emig_send_messages();

int imig_process_put(int pid, const char * name, int uid, int peer_index);

int imig_send_messages();

int imig_process_confirm(unsigned int sequence_number, int decision);

int imig_process_start_migrated_process(int pid, int peer_index);

void process_cleaner();

#ifdef __cplusplus
}
#endif


#endif

