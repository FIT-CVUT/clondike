/**
 * @file tcmi_struct.h - Declaration of extension structure for task_struct
 * 
 * 
 * 
 * 
 * 
 * 
 *
 * Date: 04/26/2005
 *
 * Author: Jan Capek
 *
 * $Id: linux-2.6.23-uml-clondike.patch,v 1.1 2009-01-20 14:22:16 andrep1 Exp $
 *
 * License....
 */

#ifndef _TCMI_STRUCT_H
#define _TCMI_STRUCT_H

/** handler method type - called when switching to migration mode. */
typedef void mig_mode_handler_t(void);

enum tcmi_task_type {
	unresolved,
	shadow,
	shadow_detached, /** A task, that was shadow once, but it is now running on CCN and so it does not have remote guest */
	guest
};

/** Compound structure that holds TCMI related information for
 * shadow/stub tasks in the task_struct. There are 3 items:
 * - migration mode handler - this allows the migration component to
 * run a specific handler for each task. With this approach we can have
 * CCN and PEN on the same node
 * - data for the handler - contains a valid pointer to any data. It
 * should be guaranteed by the component that performs task attaching,
 * that the the process will retain an extra reference. 
 * - tcmi_task - points to the tcmi task that describes either
 * a shadow or a stub task. This data is interpreted by the migration
 * mode handler.
 */
struct tcmi_struct {
	/* migration mode handler */
	mig_mode_handler_t *mig_mode_handler;
	/* data for the handler */
	void *data;
	/* tcmi task associated with the handler */
	void *tcmi_task;

	enum tcmi_task_type task_type;
};

/**
 * @}
 */
#endif /* _TCMI_STRUCT_H */