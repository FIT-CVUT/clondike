
#include "ctlfs_file_helper.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct stat st = {0};

int create_directory(const char * path){

	if (stat(path, &st) == -1) {
    	mkdir(path, 0700);
	}

	return 0;
}

int create_file(const char * filename){
	create_file_write(filename, "");
}

int create_file_write(const char * filename, const char * content){
	FILE * fd;

	if ( !(fd = fopen(filename, "w"))) {
		printf("cannot create file %s\n", filename);
		return -1;
	}

	fputs(content, fd);

	fclose(fd);
	return 0;
}

int remove_directory(const char * path){
	if (stat(path, &st) == 0) {
    	if (rmdir(path))
    		return -1;
	}
	return 0;
}

int remove_file(const char * filename){
	if (stat(filename, &st) == 0) {
    	if (unlink(filename)){
    		return -1;
    	}
	}
	return 0;
}




