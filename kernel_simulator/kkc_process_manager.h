#ifndef KKC_PROCESS_MANAGER_H
#define KKC_PROCESS_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

enum migration_state{
    MIG_PROCESS_PREPARED,
    MIG_PROCESS_NEW,
    MIG_PROCESS_REQUEST,
    MIG_PROCESS_CONFIRMED,
    MIG_PROCESS_DENIED,
    MIG_PROCESS_BEGIN,
    MIG_PROCESS_END,
    MIG_PROCESS_WORKING
};


struct mig_process {
    int pid;
    int remote_pid;
    char name[100];
    int uid;
    int peer_index;
    int migration_state;
    int return_code; //used for return code of program and decision about acceptation of migration
    int jiffies;
    unsigned int sequence_number;
};

int emig_process_migrate(unsigned int sequence_number, int peer_index);

int emig_process_put(int pid, const char * name, int uid, unsigned int seq);

int imig_process_put(int pid, const char * name, int uid, int peer_index);

int emig_send_messages();

int imig_send_messages();


#ifdef __cplusplus
}
#endif


#endif

