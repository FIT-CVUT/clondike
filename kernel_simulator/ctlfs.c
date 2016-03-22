

#include "ctlfs.h"
#include "ctlfs_file_helper.h"


/*
 * Struct of ctlfs directory

clondike
 |
 \-- ccn
    |
    +-- listening-on
    +-- mounter
    |  |
    |  +-- fs_mount
    |  +-- fs_mount_device
    |  \-- fs_mount_options
    |  
    +-- listen
    +-- stop-listen-all
    \-- stop-listen-one
 



*/

int init_ctlfs(void){
	if(ctlfs_init_dirs() < 0){
		printf("Cannot init ctlfs directories\n");
		return -1;
	}
	
	if (ctlfs_init_files() < 0){
		printf("Cannot init ctlfs files\n");
		return -1;
	}
    return 0;
}

int destroy_ctlfs(void){
	
    system("rm -r /clondike/*");
    return 0;

	if (ctlfs_stop_files() < 0){
		printf("cannot destroy files\n");
		return -1;
	}

	if(ctlfs_stop_dirs() < 0){
		printf("cannot destroy directories\n");
		return -1;
	}
    return 0;
}
int ctlfs_init_dirs(void){
	if (create_directory("/clondike/ccn")){
		printf("Cannot create directory ccn.\n");
		return -1;
	}

	if (create_directory("/clondike/ccn/listening-on")){
		printf("Cannot create directory listening-on.\n");
		return -1;
	}
    
    if (create_directory("/clondike/ccn/listening-on/listen-00")){
		printf("Cannot create directory listen-00.\n");
		return -1;
	}

	if (create_directory("/clondike/ccn/mounter")){
		printf("Cannot create directory mounter.\n");
		return -1;
	}
    
    if (create_directory("/clondike/ccn/nodes")){
		printf("Cannot create directory nodes.\n");
		return -1;
	}
    
    if (create_directory("/clondike/pen")){
		printf("Cannot create directory pen.\n");
		return -1;
	}
    
    if (create_directory("/clondike/pen/nodes")){
		printf("Cannot create directory nodes.\n");
		return -1;
	}
	
    return 0;

}

int ctlfs_init_files(void){

	if (create_file("/clondike/ccn/mounter/fs-mount")){
		printf("Cannot create file fs-mount.\n");
		return -1;
	}

	if (create_file("/clondike/ccn/mounter/fs-mount-device")){
		printf("Cannot create file fs-mount-device.\n");
		return -1;
	}

	if (create_file("/clondike/ccn/mounter/fs-mount-options")){
		printf("Cannot create file fs-mount-options.\n");
		return -1;
	}

	if (create_file("/clondike/ccn/listen")){
		printf("Cannot create file listen.\n");
		return -1;
	}

	if (create_file("/clondike/ccn/stop-listen-all")){
		printf("Cannot create file stop-listen-all.\n");
		return -1;
	}

	if (create_file("/clondike/ccn/stop-listen-one")){
		printf("Cannot create file stop-listen-one.\n");
		return -1;
	}
    
    if (create_file("/clondike/ccn/listening-on/listen-00/archname")){
		printf("Cannot create file archname.\n");
		return -1;
	}
    
    if (create_file("/clondike/ccn/listening-on/listen-00/peername")){
		printf("Cannot create file peername.\n");
		return -1;
	}
    
    if (create_file("/clondike/ccn/listening-on/listen-00/sockname")){
		printf("Cannot create file sockname.\n");
		return -1;
	}
    
    if (create_file_write("/clondike/ccn/nodes/count", "0\n")){
		printf("Cannot create file count.\n");
		return -1;
	}
    
    if (create_file_write("/clondike/pen/nodes/count", "0\n")){
		printf("Cannot create file count.\n");
		return -1;
	}

	return 0;
}

int ctlfs_stop_dirs(void){
   
    if (remove_directory("/clondike/ccn/listening-on/listen-00")){
		printf("Cannot delete directory listen-00.\n");
		return -1;
	}

	if (remove_directory("/clondike/ccn/listening-on")){
		printf("Cannot delete directory listening-on.\n");
		return -1;
	}

	if (remove_directory("/clondike/ccn/mounter")){
		printf("Cannot delete directory mounter.\n");
		return -1;
	}
    
    if (remove_directory("/clondike/ccn/nodes")){
		printf("cannot delete directory nodes.\n");
		return -1;
	}

	if (remove_directory("/clondike/ccn")){
		printf("cannot delete directory ccn.\n");
		return -1;
	}
    
    if (remove_directory("/clondike/pen/nodes")){
		printf("cannot delete directory nodes.\n");
		return -1;
	}
    
    if (remove_directory("/clondike/pen")){
		printf("cannot delete directory pen.\n");
		return -1;
	}
}

int ctlfs_stop_files(void){
	if (remove_file("/clondike/ccn/mounter/fs-mount")){
		printf("Cannot remove file fs-mount.\n");
		return -1;
	}

	if (remove_file("/clondike/ccn/mounter/fs-mount-device")){
		printf("Cannot remove file fs-mount-device.\n");
		return -1;
	}

	if (remove_file("/clondike/ccn/mounter/fs-mount-options")){
		printf("Cannot remove file fs-mount-options.\n");
		return -1;
	}

	if (remove_file("/clondike/ccn/listen")){
		printf("Cannot remove file listen.\n");
		return -1;
	}

	if (remove_file("/clondike/ccn/stop-listen-all")){
		printf("Cannot remove file stop-listen-all.\n");
		return -1;
	}

	if (remove_file("/clondike/ccn/stop-listen-one")){
		printf("Cannot remove file stop-listen-one.\n");
		return -1;
	}
    
    if (remove_file("/clondike/ccn/listening-on/listen-00/archname")){
		printf("Cannot remove file archname.\n");
		return -1;
	}
    
    if (remove_file("/clondike/ccn/listening-on/listen-00/peername")){
		printf("Cannot remove file peername.\n");
		return -1;
	}
    
    if (remove_file("/clondike/ccn/listening-on/listen-00/sockname")){
		printf("Cannot remove file sockname.\n");
		return -1;
	}
   
    if (remove_file("/clondike/ccn/nodes/count")){
		printf("Cannot remove file count.\n");
		return -1;
	}

    if (remove_file("/clondike/pen/nodes/count")){
		printf("Cannot remove file count.\n");
		return -1;
	}
	return 0;
}

int create_pen_node_directory(struct sockaddr_in * pen_node_addr, int index){
    char dir_name[100];
    
    sprintf(dir_name, "/clondike/pen/nodes/%d", index);
    if (create_directory(dir_name)){
        printf("Cannot create %s\n", dir_name);
        return -1;
    }

    sprintf(dir_name, "/clondike/pen/nodes/%d/connections", index);
    if (create_directory(dir_name)){
        printf("Cannot create %s\n", dir_name);
        return -1;
    }

    sprintf(dir_name, "/clondike/pen/nodes/%d/connections/ctrlconn", index);
    if (create_directory(dir_name)){
        printf("Cannot create %s\n", dir_name);
        return -1;
    }
    
    sprintf(dir_name, "/clondike/pen/nodes/%d/connections/ctrlconn/peername", index);
    if (create_file(dir_name)){
		printf("Cannot create %s.\n", dir_name);
		return -1;
	}
    
    char content[50];
    sprintf(content, "%s:54321\n", inet_ntoa(pen_node_addr->sin_addr));
    //sprintf(content, "%s:%d", inet_ntoa(pen_node_addr->sin_addr), ntohs(pen_node_addr->sin_port));
    create_file_write(dir_name, content);

}

void change_ccn_count(int amount){
    FILE * fd;
    int count;
    fd = fopen("/clondike/ccn/nodes/count", "r+");

    fscanf(fd, "%d", &count);
    
    printf("current count: %d", count);

    count += amount;

    fseek(fd, 0, SEEK_SET);
    
    fprintf(fd, "%d\n", count);

    fclose(fd);
}

void inc_ccn_count(){
    change_ccn_count(1);
}
void dec_ccn_count(){
    change_ccn_count(-1);
}


void change_pen_count(int amount){
    FILE * fd;
    int count;
    fd = fopen("/clondike/pen/nodes/count", "r+");

    fscanf(fd, "%d", &count);
    
    printf("current count: %d", count);

    count += amount;

    fseek(fd, 0, SEEK_SET);
    
    fprintf(fd, "%d\n", count);

    fclose(fd);
}

void inc_pen_count(){
    change_pen_count(1);
}

void dec_pen_count(){
    change_pen_count(-1);
}


