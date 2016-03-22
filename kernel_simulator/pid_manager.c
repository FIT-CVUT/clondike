#include "pid_manager.h"

static int pid = 0;

int get_next_pid(){
    return pid++;
}
