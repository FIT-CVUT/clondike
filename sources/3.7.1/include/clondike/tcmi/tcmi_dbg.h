/**
* @file tcmi_dbg.h - Helper module when debugging TCMI tasks in kernel
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

#ifndef _TCMI_DBG_H
#define _TCMI_DBG_H


extern int tcmi_dbg;

#define TCMI_ON_DEBUG if (tcmi_dbg && current->tcmi.tcmi_task)

/**
 * @}
 */

#endif /* _TCMI_DBG_H */