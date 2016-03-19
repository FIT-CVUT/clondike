#include "pen_watcher.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


int check_pen_watcher(){
    FILE * fin;
    
    fin = fopen("/clondike/pen/connect", "a");
    
    if (fin == NULL){
        printf("cannot open pen connect file\n");
        return 0;
    }

    int size = ftell(fin);
   
    fclose(fin);

    //size > 1 means file is not empty and there is not only space or newline inside
    if (size > 1)
        return 1;

    return 0;
}
