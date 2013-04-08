#ifndef _LIB_UTIL_H
#define _LIB_UTIL_H

#include <linux/pid_namespace.h>
#include <linux/namei.h>

static inline struct task_struct* task_find_by_pid(pid_t pid) {
	return pid_task(find_pid_ns(pid, current->nsproxy->pid_ns), PIDTYPE_PID);
}

/**
 * Creates a new directory. Assumes, its parent directory already exists!!
 * 
 * Fix kernel 3.7.1
 * Renamed first parameter path to name 
 */
static inline int mk_dir(const char* name, int mode) {
	struct dentry *dentry;
    struct path path;
	int err;
  
  err = kern_path(name, LOOKUP_PARENT ,&path);           // Replaced function path_lookup to kern_path for kernel 3.7.1 by Jiri Rakosnik
	if ( err ) {
		return err;
	}
  
  	  
	dentry = kern_path_create(AT_FDCWD, name, &path, 1);       // Used function kern_path_create instead of lookup_create and path_lookup, Potential problem!!!
  //dentry = lookup_create(&nd, 1);
        err = PTR_ERR(dentry);
        if (IS_ERR(dentry))
                return err;

	err = vfs_mkdir(path.dentry->d_inode, dentry, mode);
	
	return err;
}

#endif
