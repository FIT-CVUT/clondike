/**
 * @file tcmi_hooks_factory.h - Declaration of all a factory class that
 *                              produces hooks declarations and definitions
 * 
 * 
 * 
 * 
 * 
 *
 * Date: 04/21/2005
 *
 * Author: Jan Capek, based on lmmdefs.h by Martin Kacer
 *
 * $Id: linux-2.6.23-uml-clondike.patch,v 1.1 2009-01-20 14:22:16 andrep1 Exp $
 *
 * License....
 */

#ifndef _TCMI_HOOKS_FACTORY_H
#define _TCMI_HOOKS_FACTORY_H


/** @defgroup tcmi_hooks_factory_class tcmi_hooks_factory class 
 * 
 * This \<\<singleton\>\> class allows declaration and definition
 * of new kernel hooks. A component that wants to create a new hook
 * adds its definition in tcmi_hooks.h, using TCMI_HOOKS_DEFINE macro. 
 * The hook method is then called using TCMI_HOOKS_CALL macro from
 * appropriate place in the kernel. The parameters passed to the call
 * must match the hook definition.
 *
 * Any module that wants to register a method that will always be
 * called by the hook calls
 * tcmi_hooks_register_NAME(custom_method). Where the NAME suffix is
 * the identifier used in HOOK definition in tcmi_hooks.h.
 * 
 * The benefit of this solution, is that we have to export only one
 * new symbol - the hook pointer. Everything else is handled by static
 * inline methods or macros.
 *
 *
 *@{
 */

/** 
 * Defines a hook that requires:
 *
 * - declaration of new data type for the method that is to be
 * registered. This method is then required as a parameter for the
 * registration method.
 *
 * - declaration of registration/unregistration methods
 * - declares the pointer to the hooks method
 *
 * In addition, since this file is also included by tcmi_hooks.c
 * module it will define the registration and unregistration function,
 * and a the default hook method - NULL
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>

/* When used as private, we generate also definitions */
#ifdef TCMI_HOOKS_FACTORY_PRIVATE
/** 
 * Following macro is used by the public TCMI_HOOKS_DEFINE to define the actual
 * hook method pointer and export it. This is done in tcmi_hooks.c and the user
 * doesn't have to worry about it anymore as it will get linked into the kernel.
 */
#define TCMI_HOOKS_DEFINE_PRIVATE(method, args...)					\
tcmi_hooks_##method##_t *tcmi_hooks_##method = NULL;					\
EXPORT_SYMBOL(tcmi_hooks_##method);					

#else
/* empty macro, when included from some place else*/
#define TCMI_HOOKS_DEFINE_PRIVATE(method, args...)
#endif /* TCMI_HOOKS_FACTORY_PRIVATE */

/** 
 * This macro is to declare the hook method pointer in tcmi_hooks.h.
 * In the tcmi_hooks.c, it also defines the hook pointer.
 */
#define TCMI_HOOKS_DEFINE(method, args...)						\
typedef long tcmi_hooks_##method##_t(args);						\
extern tcmi_hooks_##method##_t *tcmi_hooks_##method;					\
static inline void tcmi_hooks_register_##method(tcmi_hooks_##method##_t *method)	\
{											\
	tcmi_hooks_##method = method;							\
}											\
static inline void tcmi_hooks_unregister_##method(void)					\
{											\
	tcmi_hooks_##method = NULL;							\
}											\
TCMI_HOOKS_DEFINE_PRIVATE(method, args);



/**
 * Since each hook method returns an int, this macro generates an
 * expression, that calls the hook method if one is defined.
 * The value of the expression is 0, if no hook method has been registered.
 * If there is a valid hook method registered, we get back the value
 * returned by this method. 
 *
 * @param method - name of the method(e.g. exec) that
 * is to be called
 * @param args - arguments of the method
 */
#define TCMI_HOOKS_CALL(method, args...)		\
({							\
	int __tcmi_hook_ret = 0;			\
 if (tcmi_hooks_##method != NULL)		\
		__tcmi_hook_ret =			\
			tcmi_hooks_##method(args);	\
	__tcmi_hook_ret;				\
})


#define TCMI_TASK_GUEST if (current->tcmi.tcmi_task && current->tcmi.task_type == guest)

/**
 * @}
 */
 
#endif /* _TCMI_HOOKS_FACTORY_H */