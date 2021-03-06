/**
 * @file tcmi_guest_generic_rpc.h - a RPC class used by guest process .
 *
 * Date: 3/08/2007
 *
 * Author: Petr Malat
 *
 * $Id: tcmi_guest_generic_rpc.h,v 1.1 2007/09/02 12:09:55 malatp1 Exp $
 *
 * This file is part of Task Checkpointing and Migration Infrastructure(TCMI)
 * Copyleft (C) 2007  Petr Malat
 * 
 * TCMI is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * TCMI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _TCMI_GUEST_RPC_GENERIC_H
#define _TCMI_GUEST_RPC_GENERIC_H

#include "tcmi_guest_rpc.h"
/** @defgroup tcmi_guest_rpc_generic_class Generic RPCs 
 *
 * @ingroup tcmi_guest_rpc_class
 *
 * A \<\<singleton\>\> class that is executing RPC on PEN side.
 * 
 * @{
 */

/** Creates declaration for default RPC (which takes only long arguments and return
 * only one value 
 *
 * param num - Number of arguments
 */
#define TCMI_GUEST_RPC_SYS_N_DEF(num) \
long tcmi_guest_rpc_sys_##num(unsigned, long params[TCMI_RPC_MAXPARAMS])

/** Generic guest RPC method for call with no parameters */
TCMI_GUEST_RPC_SYS_N_DEF(0);

/** Generic guest RPC method for call with one parameters */
TCMI_GUEST_RPC_SYS_N_DEF(1);

/** Generic guest RPC method for call with two parameters */
TCMI_GUEST_RPC_SYS_N_DEF(2);

/** Generic guest RPC method for call with three parameters */
TCMI_GUEST_RPC_SYS_N_DEF(3);

/**
 * @}
 */

#endif /* _TCMI_GUEST_RPC_GENERIC_H */

