#ifndef _CTLFS_FILE_HELPER_H
#define _CTLFS_FILE_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>


int create_directory(const char * path);

int create_file(const char * filename);

int create_file_write(const char * filename, const char * content);

int remove_directory(const char * path);

int remove_file(const char * filename);

#ifdef __cplusplus
}
#endif

#endif
