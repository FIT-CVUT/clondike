

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

int ctlfs_init_dirs(void){
	if (create_directory("/clondike/ccn")){
		printf("Cannot create directory ccn.\n");
		return -1;
	}

	if (create_directory("/clondike/ccn/listening-on")){
		printf("Cannot create directory listening-on.\n");
		return -1;
	}

	if (create_directory("/clondike/ccn/mounter")){
		printf("Cannot create directory mounter.\n");
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

	return 0;
}

int ctlfs_stop_dirs(void){

	if (remove_directory("/clondike/ccn/listening-on")){
		printf("Cannot delete directory listening-on.\n");
		return -1;
	}

	if (remove_directory("/clondike/ccn/mounter")){
		printf("Cannot delete directory mounter.\n");
		return -1;
	}

	if (remove_directory("/clondike/ccn")){
		printf("Cannot delete directory ccn.\n");
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

	return 0;
}


