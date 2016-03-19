#include "netlink_message.h"
#include "kkc_process_manager.h"
#include "kkc.h"
#include "kkc_messages.h"

#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;

static vector<mig_process *> emig_processes;
static vector<mig_process *> imig_processes;

int emig_process_put(int pid, const char * name, int uid, unsigned int seq){
    struct mig_process * p = (struct mig_process *) malloc(sizeof(struct mig_process));

    p->pid = pid;
    strcpy(p->name, name);
    p->uid = uid;
    p->migration_state = MIG_PROCESS_PREPARED;
    p->sequence_number = seq;
    p->jiffies = 0;
    emig_processes.push_back(p);

    cout << "push emig process" << endl;

    return 0;
}

int emig_process_migrate(unsigned int sequence_number, int peer_index){
    struct mig_process * p;
    for(vector<mig_process *>::iterator it = emig_processes.begin(); it != emig_processes.end(); it++){
        if((*it)->sequence_number == sequence_number) {
            p = *it;
            break;
        }
    }

    p->peer_index = peer_index;
    p->migration_state = MIG_PROCESS_NEW;

    cout << "process " << p->pid << " prepared for migration" << endl;
    return 0;
}

int imig_process_put(int pid, const char * name, int uid, int peer_index){
    struct mig_process * p = (struct mig_process *) malloc(sizeof(struct mig_process));
    
    p->pid = pid;
    strcpy(p->name, name);
    p->uid = uid;
    p->peer_index = peer_index;
    p->migration_state = MIG_PROCESS_NEW;

    imig_processes.push_back(p);

    return 0;
}


int emig_send_messages(){
    for(vector<mig_process *>::iterator it = emig_processes.begin(); it != emig_processes.end(); it++){
        switch ((*it)->migration_state){
            case MIG_PROCESS_NEW:
                kkc_send_emig_request((*it)->peer_index, (*it)->pid, (*it)->uid, (*it)->name);
                (*it)->migration_state = MIG_PROCESS_REQUEST;
                break;
            case MIG_PROCESS_CONFIRMED:
                kkc_send_emig_begin((*it)->peer_index, (*it)->pid, (*it)->uid, (*it)->name);
                (*it)->migration_state = MIG_PROCESS_BEGIN;
                break;
            case MIG_PROCESS_END:
                //TODO nothing
                break;
        }
    }
    return 0;
}


int imig_send_messages(){
    for(vector<mig_process *>::iterator it = imig_processes.begin(); it != imig_processes.end(); it++){
        switch ((*it)->migration_state){
            case MIG_PROCESS_NEW:
                if( netlink_send_immigration_request((*it)->uid, (*it)->pid, 
                            (*it)->peer_index, (*it)->name, (*it)->jiffies) != 0){
                    cout << "cannot send immigration request message" << endl;
                }
                (*it)->migration_state = MIG_PROCESS_REQUEST;
                break;
            case MIG_PROCESS_CONFIRMED:
                kkc_send_emig_request_response((*it)->peer_index, (*it)->pid, (*it)->return_code);
                break;
            case MIG_PROCESS_BEGIN:
                if(netlink_send_immigration_confirmed((*it)->uid, (*it)->pid, (*it)->peer_index, (*it)->name, (*it)->jiffies, (*it)->remote_pid) != 0){
                    cout << "cannot send immigration confirmed message" << endl;
                }
                //TODO:
                //work(*it);
                (*it)->migration_state = MIG_PROCESS_WORKING;
                break;
                    
            case MIG_PROCESS_END:
                kkc_send_emig_done((*it)->peer_index, (*it)->pid, (*it)->return_code);
                break;
        }
    }
    return 0;
}




