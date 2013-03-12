/**
 * @file tcmi_hooks.h - Declaration of all hooks that are to be used in the kernel.
 * 
 * 
 * 
 * 
 * 
 * 
 *
 * Date: 04/21/2005
 *
 * Author: Jan Capek
 *
 * $Id: linux-2.6.23-uml-clondike.patch,v 1.1 2009-01-20 14:22:16 andrep1 Exp $
 *
 * License....
 */

#ifndef _TCMI_HOOKS_H
#define _TCMI_HOOKS_H

#include <clondike/tcmi/tcmi_hooks_factory.h>
#include <asm/ptrace.h>
#include <asm/siginfo.h>
#include <linux/capability.h>
#include <linux/resource.h>

/** execve */
TCMI_HOOKS_DEFINE(execve, const char *, const char * const*, const char * const*, struct pt_regs *);

/** wait */
TCMI_HOOKS_DEFINE(sys_wait4, pid_t, int __user *, int, struct rusage __user *);

/** fork hooks */
/** Called in the beginning of the fork */
TCMI_HOOKS_DEFINE(pre_fork, unsigned long, unsigned long, struct pt_regs *, unsigned long, int __user *, int __user *);
/** Called in the middle of fork (used to attach shadow/guest task to a newly forked task). Takes new child as a param */
TCMI_HOOKS_DEFINE(in_fork, struct task_struct*);
/** 
 * Called in the end of the fork. Takes as param return value of fork, pid of process forked on associated CCN + result buffers (so that
 * we can reset them, in case fork failed and we've filled them in prefork.
 *
 * The method is called after succesful fork, but befor the process is actually started
 */
TCMI_HOOKS_DEFINE(post_fork, struct task_struct*, long, pid_t, int __user *, int __user *);

/** exit hook */
TCMI_HOOKS_DEFINE(exit, long);

/** syscalls **/
/** signal */
TCMI_HOOKS_DEFINE(sys_kill, int, int);
TCMI_HOOKS_DEFINE(do_tkill, int, int, int);
TCMI_HOOKS_DEFINE(sys_rt_sigqueueinfo, int, int, siginfo_t*);

/** pid, gid and session manipulation */
TCMI_HOOKS_DEFINE(sys_getpid, void);
TCMI_HOOKS_DEFINE(sys_getppid, void);
TCMI_HOOKS_DEFINE(sys_getpgid, pid_t);
TCMI_HOOKS_DEFINE(sys_setpgid, pid_t, pid_t);
TCMI_HOOKS_DEFINE(sys_getsid, pid_t);
TCMI_HOOKS_DEFINE(sys_setsid, void);
TCMI_HOOKS_DEFINE(sys_getpgrp, void);

/** user identification */
TCMI_HOOKS_DEFINE(sys_geteuid, void);
TCMI_HOOKS_DEFINE(sys_getuid, void);
TCMI_HOOKS_DEFINE(sys_getresuid, uid_t*, uid_t*, uid_t*);
TCMI_HOOKS_DEFINE(sys_setresuid, uid_t, uid_t, uid_t);
TCMI_HOOKS_DEFINE(sys_setuid, uid_t);
TCMI_HOOKS_DEFINE(sys_setreuid, uid_t, uid_t);

/** group identification */
TCMI_HOOKS_DEFINE(sys_getegid, void);
TCMI_HOOKS_DEFINE(sys_getgid, void);
TCMI_HOOKS_DEFINE(sys_getgroups, int, gid_t *);
TCMI_HOOKS_DEFINE(sys_getresgid, gid_t*, gid_t*, gid_t*);
TCMI_HOOKS_DEFINE(sys_setgid, gid_t);
TCMI_HOOKS_DEFINE(sys_setregid, gid_t, gid_t);
TCMI_HOOKS_DEFINE(sys_setresgid, gid_t, gid_t, gid_t);
TCMI_HOOKS_DEFINE(sys_setgroups, int, gid_t *);

/** other */
TCMI_HOOKS_DEFINE(sys_capget, cap_user_header_t, cap_user_data_t);


/** SIGUNUSED default signal handler hook */
TCMI_HOOKS_DEFINE(sig_unused, struct pt_regs*);
TCMI_HOOKS_DEFINE(sig_deliver, int);
TCMI_HOOKS_DEFINE(sig_delivered, int);
TCMI_HOOKS_DEFINE(did_stop, int);
TCMI_HOOKS_DEFINE(group_stop, int);
TCMI_HOOKS_DEFINE(deq_sig, int);
TCMI_HOOKS_DEFINE(send_sig, int);
TCMI_HOOKS_DEFINE(doing_sigfatal, int, int);


TCMI_HOOKS_DEFINE(replace_proc_self_file, const char*, const char**);

/**
 * @}
 */

#endif /* _TCMI_HOOKS_H */