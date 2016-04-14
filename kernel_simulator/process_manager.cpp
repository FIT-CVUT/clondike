#include "netlink_message.h"
#include "process_manager.h"
#include "kkc.h"
#include "kkc_messages.h"
#include "message_helper.h"
#include "pid_manager.h"
#include "worker.h"
#include "fifo_reader.h"

#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;

static vector<mig_process *> emig_processes;
static vector<mig_process *> imig_processes;

int emig_process_put(int pid, const char * name, int uid, unsigned int seq, uint64_t jiff, int process_fd){
    struct mig_process * p = (struct mig_process *) malloc(sizeof(struct mig_process));

    p->pid = pid;
    strcpy(p->name, name);
    p->uid = uid;
    p->migration_state = MIG_PROCESS_PREPARED;
    p->sequence_number = seq;
    p->jiffies = jiff;
    p->process_fd = process_fd;
    emig_processes.push_back(p);

    cout << "push emig process" << endl;

    return 0;
}

int emig_process_migrate(unsigned int sequence_number, int peer_index){
    struct mig_process * p = NULL;
    for(vector<mig_process *>::iterator it = emig_processes.begin(); it != emig_processes.end(); it++){
        if((*it)->sequence_number == sequence_number) {
            p = *it;
            break;
        }
    }
    if (!p)
        return -1;

    p->peer_index = peer_index;
    p->migration_state = MIG_PROCESS_NEW;

    cout << "process " << p->pid << " prepared for migration" << endl;
    return 0;
}

int emig_process_denied(unsigned int sequence_number){
    struct mig_process * p = NULL;
    for(vector<mig_process *>::iterator it = emig_processes.begin(); it != emig_processes.end(); it++){
        if((*it)->sequence_number == sequence_number) {
            p = *it;
            break;
        }
    }
    if (!p)
        return -1;

    p->migration_state = MIG_PROCESS_DENIED;
    cout << "process " << p->pid << " will run locally" << endl;
    return 0;
}

int emig_process_migration_confirmed(int pid, int decision){
    struct mig_process * p;
    for(vector<mig_process *>::iterator it = emig_processes.begin(); it != emig_processes.end(); it++){
        if((*it)->pid == pid) {
            p = *it;
            break;
        }
    }

    if (decision == MIGRATE){
        p->migration_state = MIG_PROCESS_CONFIRMED;
        cout << "process " << p->pid << " confirmed" << endl;
    }
    else{
        p->migration_state = MIG_PROCESS_DENIED;
        cout << "emigration process " << p->pid << " denied" << endl;
    }

    return 0;
}

int emig_process_done(int pid, int return_code){
    struct mig_process * p;
    for(vector<mig_process *>::iterator it = emig_processes.begin(); it != emig_processes.end(); it++){
        if((*it)->pid == pid) {
            p = *it;    
        }
    }

    p->return_code = return_code;
    p->migration_state = MIG_PROCESS_END;
    return 0;
}

int imig_process_put(int pid, const char * name, int uid, int peer_index, uint64_t jiff){
    struct mig_process * p = (struct mig_process *) malloc(sizeof(struct mig_process));
    
    p->pid = pid;
    strcpy(p->name, name);
    p->uid = uid;
    p->peer_index = peer_index;
    p->migration_state = MIG_PROCESS_NEW;
    p->jiffies = jiff;
    p->process_fd = -1;

    imig_processes.push_back(p);

    return 0;
}

int imig_process_confirm(unsigned int sequence_number, int decision){
    struct mig_process * p = NULL;
    for(vector<mig_process *>::iterator it = imig_processes.begin(); it != imig_processes.end(); it++){
        if((*it)->sequence_number == sequence_number) {
            p = *it;
            break;
        }
    }
    if (!p)
        return -1;

    if (decision == MIGRATE)
        p->migration_state = MIG_PROCESS_CONFIRMED;
    else
        p->migration_state = MIG_PROCESS_DENIED;
    p->return_code = decision;

    cout << "process " << p->pid << " confirmed" << endl;
    return 0;
}

int imig_process_start_migrated_process(int pid, int peer_index){
    struct mig_process * p = NULL;
    for(vector<mig_process *>::iterator it = imig_processes.begin(); it != imig_processes.end(); it++){
        if((*it)->pid == pid && (*it)->peer_index == peer_index) {
            p = *it;
            break;
        }
    }
    if (!p)
        return -1;


    p->remote_pid = get_next_pid();
    p->migration_state = MIG_PROCESS_BEGIN;
    
    //notify userspace about new process - imigrated process
    netlink_send_task_fork(p->remote_pid, get_next_pid());
    
    return 0;
}

int emig_send_messages(){
    for(vector<mig_process *>::iterator it = emig_processes.begin(); it != emig_processes.end(); it++){
        switch ((*it)->migration_state){
            case MIG_PROCESS_NEW:
                kkc_send_emig_request((*it)->peer_index, (*it)->pid, (*it)->uid, (*it)->name, (*it)->jiffies);
                (*it)->migration_state = MIG_PROCESS_REQUEST;
                break;
            case MIG_PROCESS_CONFIRMED:
                kkc_send_emig_begin((*it)->peer_index, (*it)->pid, (*it)->uid, (*it)->name);
                (*it)->migration_state = MIG_PROCESS_BEGIN;
                break;
            case MIG_PROCESS_DENIED:
                cout << "migration process denied" << endl;
                //have to run localy
                (*it)->migration_state = MIG_PROCESS_WORKING_LOCALY;
                fork_and_work(*it);
                break;
            case MIG_PROCESS_END:
                netlink_send_task_exit((*it)->pid, (*it)->return_code);
                send_process_exit((*it)->process_fd, (*it)->return_code);
                (*it)->migration_state = MIG_PROCESS_CLEAN;
                break;
        }
    }
    return 0;
}


int imig_send_messages(){
    for(vector<mig_process *>::iterator it = imig_processes.begin(); it != imig_processes.end(); it++){
        switch ((*it)->migration_state){
            case MIG_PROCESS_NEW:
		cout << "new immigration process " << (*it)->pid << ", sending netlink" << endl;
                if( netlink_send_immigration_request((*it)->uid, (*it)->pid, 
                            (*it)->peer_index, (*it)->name, (*it)->jiffies) != 0){
                    cout << "cannot send immigration request message" << endl;
                }
                (*it)->migration_state = MIG_PROCESS_REQUEST;
                (*it)->sequence_number = get_sequence_number();
                break;
            case MIG_PROCESS_CONFIRMED:
		cout << "process confirmed " << (*it)->pid << ", sending emig_request_response" << endl;
                kkc_send_emig_request_response((*it)->peer_index, (*it)->pid, (*it)->return_code);
                (*it)->migration_state = MIG_PROCESS_CONFIRMED_SEND;
                break;
            case MIG_PROCESS_DENIED:
		cout << "process denied " << (*it)->pid << ", sending emig_request_response" << endl;
                kkc_send_emig_request_response((*it)->peer_index, (*it)->pid, (*it)->return_code);
                (*it)->migration_state = MIG_PROCESS_CLEAN;
                break;
            case MIG_PROCESS_BEGIN:
		cout << "process begin " << (*it)->pid << ", sending netlink img_confirmed" << endl;
                if(netlink_send_immigration_confirmed((*it)->uid, (*it)->remote_pid, (*it)->peer_index, (*it)->name, (*it)->jiffies, (*it)->pid) != 0){
                    cout << "cannot send immigration confirmed message" << endl;
                }
                (*it)->migration_state = MIG_PROCESS_WORKING;
                fork_and_work(*it);
                break;
                    
            case MIG_PROCESS_END:
		cout << "process end " << (*it)->pid << ", sending netlink and emig_done" << endl;
                kkc_send_emig_done((*it)->peer_index, (*it)->pid, (*it)->return_code);
                netlink_send_task_exit((*it)->remote_pid, (*it)->return_code);
                (*it)->migration_state = MIG_PROCESS_CLEAN;
                break;
        }
    }
    return 0;
}

void process_cleaner(){
    int clean = 1;
    
    while(clean){
        for(vector<mig_process *>::iterator it = emig_processes.begin(); it != emig_processes.end(); it++){
            if((*it)->migration_state == MIG_PROCESS_CLEAN) {
                free(*it);
                emig_processes.erase(it);
                break;
            }
        }
        clean = 0;
    }

    while(clean){
        for(vector<mig_process *>::iterator it = imig_processes.begin(); it != imig_processes.end(); it++){
            if((*it)->migration_state == MIG_PROCESS_CLEAN) {
                free(*it);
                imig_processes.erase(it);
                break;
            }
        }
        clean = 0;
    }
}


